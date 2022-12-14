#pragma once

namespace jpd
{
    ThreadPool::ThreadPool(const size_t ThreadCount) noexcept :
        m_AvailableThreads{ ComputeThreadCount(ThreadCount) }
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

    template <typename Func, typename... Args, typename ReturnType>
    inline [[nodiscard]]
    std::future<ReturnType> ThreadPool::QueueFunction(Func&& F, Args&&... args) noexcept
    {
        std::function<ReturnType()> Task = std::bind( std::forward<Func>(F)
                                                    , std::forward<Args>(args)... );
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

    template <typename Func, typename... Args, typename ReturnType>
    inline [[nodiscard]]
    GroupTasks<ReturnType> ThreadPool::QueueAndPartitionLoop(const size_t StartIndex, const size_t EndIndex, const size_t PartitionCount, Func&& F, Args&&... args) noexcept
    {
        assert(PartitionCount > 0);

        // Assign Relevant Number Of Partitions
        GroupTasks<ReturnType> TaskFutures(PartitionCount);
        PartitionTasks PartitionData(ComputeThreadCount(PartitionCount), StartIndex, EndIndex);
        auto StartIndices = PartitionData.PartitionLoopIndices();

        for (size_t i = 0, max = StartIndices.size(); i < (max - 1); ++i)
        {
            TaskFutures[i] = QueueFunction( std::forward<Func>(F)
                                          , StartIndices[i]
                                          , StartIndices[i + 1]
                                          , std::forward<Args>(args)... );
        }

        TaskFutures[StartIndices.size() - 1] = QueueFunction( std::forward<Func>(F)
                                                            , StartIndices.back()
                                                            , EndIndex
                                                            , std::forward<Args>(args)... );

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