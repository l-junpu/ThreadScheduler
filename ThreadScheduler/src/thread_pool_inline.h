#pragma once

namespace jpd
{
    ThreadPool::ThreadPool(const size_t ThreadCount, const size_t MinimumPartitionSize) noexcept :
        m_AvailableThreads{ ComputeThreadCount(ThreadCount) }
    ,   m_MinPartitionSize{ MinimumPartitionSize }
    {
        CreateThreads();
    }

    ThreadPool::~ThreadPool() noexcept
    {
        WaitForAllTasks();
        DestroyThreads();
    }

    inline [[nodiscard]]
    size_t ThreadPool::GetTotalTaskCount(void) const noexcept
    {
        return m_TaskQueue.size();
    }

    inline [[nodiscard]]
    size_t ThreadPool::GetActiveTaskCount(void) const noexcept
    {
        return m_TotalTaskCount - m_TaskQueue.size();
    }

    template <typename Func, typename... Args>
    inline [[nodiscard]]
    void ThreadPool::QueueTask(Func&& F, Args&&... args) noexcept
    {
        VoidFunc Task = std::bind( std::forward<Func>(F)
                                 , std::forward<Args>(args)... );
        {
            std::scoped_lock ScopeLock(m_MutexLock);
            m_TaskQueue.push(Task);
        }
        ++m_TotalTaskCount;
        m_CVNewTask.notify_one();
    }

    // test
    template <typename... T_Args, typename... T_Values, typename T_Concat_Tuple = void>
    inline std::tuple<T_Args...> ReturnValidType(std::tuple<T_Args...>* Args, std::tuple<T_Values...>* Values, T_Concat_Tuple* Output, const size_t Index)
    {
        using FuncParamType  = std::tuple<T_Args...>;
        using InputParamType = std::tuple<T_Values...>;

        using ExtractedType = Extract<Index, FuncParamType>::Type;
        using InputType     = Extract<Index, InputParamType>::Type;

        // Index >= 1
        if (Output)
        {
            auto CatTuple = std::tuple_cat(*Output, std::is_lvalue_reference_v<ExtractedType> ? std::ref(std::get<Index>(*Values))
                                                                                              : std::forward<InputType>(std::get<Index>(Values)) );

            if (Index + 1 == sizeof...(T_Args))
            {
                PrintTypes(std::cout, CatTuple);
                return std::move(CatTuple);
            }
            else
            {
                return ReturnValidType(Args, Values, &CatTuple, Index+1);
            }
        }
        // Index == 0
        else
        {
            assert(Index == 0);

            auto BaseTuple = std::make_tuple<ExtractedType>( std::is_lvalue_reference_v<ExtractedType> ? std::ref(std::get<Index>(*Values))
                                                                                                       : std::forward<InputType>(std::get<Index>(Values)) );
            return ReturnValidType(Args, Values, &BaseTuple, Index+1);
        }
    }

    template <typename... T_Args, typename... T_Values>
    inline std::tuple<T_Args...> ReturnValidType(std::tuple<T_Args...>* Args, std::tuple<T_Values...>* Values)
    {
        assert( sizeof...(T_Args) == sizeof...(T_Values) );
        int* a = nullptr;
        return ReturnValidType(Args, Values, a, 0);
    }

    template <typename... Args>
    void PrintTypes(std::ostream& out, Args&&... args)
    {
        out << "\n\n";
        (( out << "Type: " << typeid(args).name() ), ...);
        out << "\n\n";
    }
    template <typename... Args>
    void PrintTypes(std::ostream& out, std::tuple<Args...>&)
    {
        out << "\n\n";
        ((out << "Type: " << typeid(Args).name()), ...);
        out << "\n\n";
    }
    template <typename... Args>
    void PrintTypes(std::ostream& out)
    {
        out << "\n\n";
        ((out << "Type: " << typeid(Args).name()), ...);
        out << "\n\n";
    }
    // test

    template <typename Func, typename... T_Args, typename ReturnType>
    inline [[nodiscard]]
    std::future<ReturnType> ThreadPool::QueueFunction(Func&& F, T_Args&&... Args) noexcept
    {
        std::function<ReturnType()> Task = std::bind( std::forward<Func>(F)
                                                    , std::forward<T_Args>(Args)... );
        std::shared_ptr<std::promise<ReturnType>> TaskPromise = std::make_shared<std::promise<ReturnType>>();

        QueueTask( [Task, TaskPromise]()
                   {
                       try
                       {
                           if constexpr (std::is_same_v<ReturnType, void>)
                           {
                               std::invoke(Task);
                               TaskPromise->set_value();
                           }
                           else
                           {
                               TaskPromise->set_value(std::invoke(Task));
                           }
                       }
                       catch (std::exception& e)
                       {
                           std::cout << "Exception Occurred (QueueTask): " << e.what() << std::endl;
                           TaskPromise->set_exception(std::current_exception());
                       }
                       catch (...)
                       {
                           TaskPromise->set_exception(std::current_exception());
                       }
                   });

        return TaskPromise->get_future();
    }

    template <typename Func, typename... Args, typename ReturnType>
    inline [[nodiscard]]
    GroupTasks<ReturnType> ThreadPool::QueueAndPartitionLoop(const size_t EndIndex, const size_t PartitionCount, Func&& F, Args&&... args) noexcept
    {
        assert(PartitionCount > 0);

        return QueueAndPartitionLoop(0, EndIndex, PartitionCount, std::forward<Func>(F), std::forward<Args>(args)...);
    }

    template <typename Func, typename... T_Args, typename ReturnType>
    inline [[nodiscard]]
    GroupTasks<ReturnType> ThreadPool::QueueAndPartitionLoop(const size_t StartIndex, const size_t EndIndex, const size_t PartitionCount, Func&& F, T_Args&&... Args) noexcept
    {
        assert(PartitionCount > 0);

        using Details = Traits<decltype(F)>;
        
        PrintTypes(std::cout, std::forward<T_Args>(Args)...);
        PrintTypes<Details::Args_T>(std::cout);
        std::tuple<T_Args...> ParamsToConvertTuple = std::make_tuple<T_Args...>( std::forward<T_Args>(Args)... );
        std::tuple<Details::Args_T> FnInputParams = ReturnValidType(null_tuple_v<Details::Args_T>, &ParamsToConvertTuple);

        // Assign Relevant Number Of Partitions
        GroupTasks<ReturnType> TaskFutures(PartitionCount);
        PartitionTasks PartitionData(ComputeThreadCount(PartitionCount), StartIndex, EndIndex);
        auto StartIndices = PartitionData.PartitionLoopIndices();

        for (size_t i = 0, max = StartIndices.size(); i < (max - 1); ++i)
        {
            TaskFutures[i] = QueueFunction( std::forward<Func>(F)
                                          , StartIndices[i]
                                          , StartIndices[i + 1]
                                          , std::forward<T_Args>(Args)... );
        }

        TaskFutures[StartIndices.size() - 1] = QueueFunction( std::forward<Func>(F)
                                                            , StartIndices.back()
                                                            , EndIndex
                                                            , std::forward<T_Args>(Args)... );

        return TaskFutures;
    }

    inline
    void ThreadPool::WaitForAllTasks(void) noexcept
    {
        m_Waiting = true;
        std::unique_lock<std::mutex> LockThreads(m_MutexLock);
        m_CVTaskCompleted.wait(LockThreads, [this]
                                            {
                                                return m_TotalTaskCount == (m_Paused ? m_TaskQueue.size() : 0);
                                            });
        m_Waiting = false;
    }

    inline
    void ThreadPool::ResetThreads(const size_t ThreadCount) noexcept
    {
        const bool PauseStatus = m_Paused;
        m_Paused = true;
        WaitForAllTasks();
        DestroyThreads();
        m_AvailableThreads = ComputeThreadCount(ThreadCount);
        CreateThreads();
        m_Paused = PauseStatus;
    }







    inline
    void ThreadPool::CreateThreads(void) noexcept
    {
        m_Running = true;
        m_Threads = std::make_unique<std::thread[]>(m_AvailableThreads);

        for (size_t i = 0; i < m_AvailableThreads; ++i)
        {
            m_Threads[i] = std::thread(&ThreadPool::WorkerThread, this);
        }
    }

    inline
    void ThreadPool::DestroyThreads(void) noexcept
    {
        m_Running = false;
        m_CVNewTask.notify_all();

        for (size_t i = 0; i < m_AvailableThreads; ++i)
        {
            m_Threads[i].join();
        }
    }

    inline [[nodiscard]]
    size_t ThreadPool::ComputeThreadCount(const size_t ThreadCount) noexcept
    {
        return ThreadCount == 0 || ThreadCount > std::thread::hardware_concurrency() ? std::thread::hardware_concurrency()
                                                                                     : ThreadCount;
    }

    inline
    void ThreadPool::WorkerThread(void) noexcept
    {
        while (m_Running)
        {
            VoidFunc Task;
            std::unique_lock<std::mutex> LockTask(m_MutexLock);
            m_CVNewTask.wait(LockTask, [this]{ return (!m_TaskQueue.empty() || !m_Running); });

            if (m_Running && !m_Paused)
            {
                Task = m_TaskQueue.front();
                m_TaskQueue.pop();

                LockTask.unlock();
                Task();
                LockTask.lock();

                --m_TotalTaskCount;
                if (m_Waiting)
                {
                    m_CVTaskCompleted.notify_one();
                }
            }
        }
    }
}