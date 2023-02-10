#pragma once

namespace jpd
{
    class [[nodiscard]] ThreadPool final
    {
    public:

        using VoidFunc = std::function<void()>;

        /*
            Public Member Functions
        */
        explicit ThreadPool( const size_t ThreadCount = 0
                           , const size_t MinimumPartitionSize = 0 ) noexcept;

        ~ThreadPool() noexcept;

        template < typename    Func
                 , typename... T_Args
                 , typename    ReturnType = std::invoke_result_t < std::decay_t<Func>, T_Args...> >
        inline [[nodiscard]]
        std::future<ReturnType> QueueFunction( Func&&      F
                                             , T_Args&&... Args ) noexcept;

        template < typename    Func
                 , typename... T_Args
                 , typename    ReturnType = std::invoke_result_t<std::decay_t<Func>, size_t, size_t, T_Args...> >
        inline [[nodiscard]]
        GroupTasks<ReturnType> QueueAndPartitionLoop( const size_t EndIndex
                                                    , const size_t PartitionCount
                                                    , const size_t MinPartitionSize
                                                    , Func&&       F
                                                    , T_Args&&...  Args ) noexcept;

        template < typename    Func
                 , typename... T_Args
                 , typename    ReturnType = std::invoke_result_t<std::decay_t<Func>, size_t, size_t, T_Args...> >
        inline [[nodiscard]]
        GroupTasks<ReturnType> QueueAndPartitionLoop( const size_t StartIndex
                                                    , const size_t EndIndex
                                                    , const size_t PartitionCount
                                                    , const size_t MinPartitionSize
                                                    , Func&&       F
                                                    , T_Args&&...  Args ) noexcept;

        inline
        void WaitForAllTasks(void) noexcept;

        inline
        void ResetThreads(const size_t ThreadCount = 0) noexcept;

        inline [[nodiscard]]
        size_t GetTotalTaskCount( void ) const noexcept;

        inline [[nodiscard]]
        size_t GetActiveTaskCount( void ) const noexcept;

    private:

        /*
            Private Member Functions
        */
        inline
        void CreateThreads(void) noexcept;

        inline
        void DestroyThreads(void) noexcept;

        template < typename    Func
                 , typename... T_Args
                 , typename    ReturnType = std::invoke_result_t < std::decay_t<Func>, T_Args...> >
        inline
        void QueueTask( Func&&      F
                      , T_Args&&... Args ) noexcept;

        inline [[nodiscard]]
        size_t ComputeThreadCount(const size_t ThreadCount) noexcept;

        inline
        void WorkerThread(void) noexcept;


        /*
            Partition Helper Functions
        */
        template <typename Container>
        requires( std::ranges::contiguous_range<Container> )
        inline [[nodiscard]]
        std::vector<size_t> PartitionData( const Container& Data
                                         , const size_t PartitionCount ) noexcept;

        inline [[nodiscard]]
        std::vector<size_t> PartitionData( const size_t DataCount
                                         , const size_t PartitionCount ) noexcept;

        inline [[nodiscard]]
        std::vector<size_t> PartitionLoopIndices( size_t       StartIndex
                                                , size_t       EndIndex
                                                , const size_t PartitionCount
                                                , const size_t MinimumPartitionSize = 0 ) noexcept;


        /*
            Variables
        */
        size_t                          m_AvailableThreads  = std::thread::hardware_concurrency();  // Number Of Threads Allocated To Thread Scheduler
        size_t                          m_MinPartitionSize  = 25;                                   // Minimum Number Of Elements In Each Partition - Reduces Number Of Tasks If Unnecessary
        std::mutex                      m_MutexLock         = {};                                   // Assists With Locking Of Data Accessed In Different Threads
        std::atomic_bool                m_Running           = false;                                // Controls Task Queue - Runs Task from m_TaskQueue If m_Running == True
        std::atomic_bool                m_Waiting           = false;                                // Controls Task Queue - Halts All Tasks from m_TaskQueue If m_Waiting == True
        std::atomic_bool                m_Paused            = false;                                // Controls Task Queue - Halts All Tasks (Only When Resetting Thread Pool)
        std::atomic_int32_t             m_TotalTaskCount    = 0;                                    // Tracks Total Number Of Active Tasks - TaskQueue + CurrentlyExecuting
        std::condition_variable         m_CVNewTask         = {};                                   // Enables Worker Thread Whenever A Task Is Available And Running
        std::condition_variable         m_CVTaskCompleted   = {};                                   // Notifies Main Thread Each Time A Task Is Completed If User Is Waiting For Current Tasks - Unwaits When Queued Tasks Are Completed
        std::queue<VoidFunc>            m_TaskQueue         = {};                                   // Stores Each Task Assigned To Thread Scheduler
        std::unique_ptr<std::thread[]>  m_Threads           = nullptr;                              // Stores All Worker Threads
    };
}