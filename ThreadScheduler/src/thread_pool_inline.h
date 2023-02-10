#pragma once

namespace jpd
{
    /*
        Public Member Functions
    */
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

    template <typename Func, typename... T_Args, typename ReturnType>
    inline [[nodiscard]]
    std::future<ReturnType> ThreadPool::QueueFunction(Func&& F, T_Args&&... Args) noexcept
    {
        std::function<ReturnType()> Task = std::bind( std::forward<Func>(F)
                                                    , std::forward<T_Args>(Args)... );
        auto TaskPromise = std::make_shared<std::promise<ReturnType>>();

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
    GroupTasks<ReturnType> ThreadPool::QueueAndPartitionLoop(const size_t EndIndex, const size_t PartitionCount, const size_t MinPartitionSize, Func&& F, Args&&... args) noexcept
    {
        assert(PartitionCount > 0);

        return QueueAndPartitionLoop(0, EndIndex, PartitionCount, std::forward<Func>(F), std::forward<Args>(args)...);
    }

    template <typename Func, typename... T_Args, typename ReturnType>
    inline [[nodiscard]]
    GroupTasks<ReturnType> ThreadPool::QueueAndPartitionLoop(const size_t StartIndex, const size_t EndIndex, const size_t PartitionCount, const size_t MinPartitionSize, Func&& F, T_Args&&... Args) noexcept
    {
        assert(PartitionCount > 0);

        auto StartIndices = PartitionLoopIndices( StartIndex, EndIndex, ComputeThreadCount(PartitionCount), MinPartitionSize ? MinPartitionSize : m_MinPartitionSize);
        // Assign Relevant Number Of Partitions
        GroupTasks<ReturnType> TaskFutures( StartIndices.size() - 1 );


        for (size_t i = 0, max = StartIndices.size(); i < (max - 1); ++i)
        {
            TaskFutures[i] = QueueFunction( std::forward<Func>(F)
                                          , StartIndices[i]
                                          , StartIndices[i + 1]
                                          , std::forward<T_Args>(Args)... );
        }

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




    /*
        Private Member Functions
    */
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

    template <typename Func, typename... T_Args, typename ReturnType>
    inline
    void ThreadPool::QueueTask(Func&& F, T_Args&&... Args) noexcept
    {
        VoidFunc Task = std::bind( std::forward<Func>(F)
                                 , std::forward<T_Args>(Args)... );

        BEGIN_SCOPE_LOCK(m_MutexLock);
            m_TaskQueue.push(Task);
        END_SCOPE_LOCK()

        ++m_TotalTaskCount;
        m_CVNewTask.notify_one();
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




    /*
        Partition Helper Functions
    */
    template <typename Container>
    requires( std::ranges::contiguous_range<Container> )
    inline [[nodiscard]]
    std::vector<size_t> ThreadPool::PartitionData(const Container& Data, const size_t PartitionCount) noexcept
    {
        return PartitionData(Data.size(), PartitionCount);
    }

    inline [[nodiscard]]
    std::vector<size_t> ThreadPool::PartitionData(const size_t DataCount, const size_t PartitionCount) noexcept
    {
        // There Should Be Elements To Partition
        assert(DataCount > 0);

        size_t Block = (DataCount / PartitionCount);
        float  FBlock = (DataCount / static_cast<float>(PartitionCount));

        size_t MainBlock = Block != FBlock ? Block + 1
            : Block;
        size_t LastBlock = DataCount - (MainBlock * (PartitionCount - 1));

        std::vector<size_t> PartitionedGroupSize(PartitionCount, MainBlock);
        PartitionedGroupSize[PartitionedGroupSize.size() - 1] = LastBlock;

        // Returns The Size Of Each Partitioned Data Set
        return PartitionedGroupSize;
    }

    inline [[nodiscard]]
    std::vector<size_t> ThreadPool::PartitionLoopIndices(size_t StartIndex, size_t EndIndex, const size_t PartitionCount, const size_t MinimumPartitionSize) noexcept
    {
        if (EndIndex < StartIndex)
        {
            std::swap(StartIndex, EndIndex);
        }

        size_t DataCount = EndIndex - StartIndex;
        size_t Block = MinimumPartitionSize ? MinimumPartitionSize
                                            : ( DataCount / PartitionCount );
        float  FBlock = MinimumPartitionSize ? static_cast<float>(MinimumPartitionSize)
                                             : (DataCount / static_cast<float>(PartitionCount));

        size_t MainBlockSize = (Block != FBlock) && (Block < DataCount) ? Block + 1 // might need update this
                                                                        : Block;

        std::vector<size_t> PartitionedGroupSize(std::ceil(DataCount / static_cast<float>(MainBlockSize)) + 1);

        for (size_t i = 0, max = PartitionedGroupSize.size(); i < max - 1; ++i)
        {
            PartitionedGroupSize[i] = StartIndex + i * MainBlockSize;
        }
        PartitionedGroupSize[PartitionedGroupSize.size() - 1] = EndIndex;

        // Returns A Vector Of Starting Indices Of The For Loop
        return PartitionedGroupSize;
    }
}