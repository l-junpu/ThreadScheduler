#pragma once

namespace jpd
{
    template <class, class Enable = void> struct is_iterator : std::false_type {};
    template <typename T> 
    struct is_iterator
    <T, 
     typename std::enable_if<
        std::is_base_of<std::input_iterator_tag, typename std::iterator_traits<T>::iterator_category>::value ||
        std::is_same<std::output_iterator_tag, typename std::iterator_traits<T>::iterator_category>::value 
     >::type> 
     : std::true_type {};

    template <typename T>
    using is_iterator_t = typename is_iterator<T>::value;



    class [[nodiscard]] ThreadPool final
    {
    public:

        using VoidFunc = std::function<void()>;

        /*
            Public Member Functions
        */
        explicit ThreadPool(const size_t ThreadCount = 0) noexcept;

        ~ThreadPool() noexcept;

        inline [[nodiscard]]
        size_t GetTotalTaskCount(void) const noexcept;

        inline [[nodiscard]]
        size_t GetActiveTaskCount(void) const noexcept;

        template <typename Func, typename... Args>
        inline [[nodiscard]]
        void QueueTask(Func&& F, Args&&... args) noexcept;

        template <typename Func, typename... Args, typename ReturnType = std::invoke_result_t<std::decay_t<Func>, std::decay_t<Args>...>>
        inline [[nodiscard]]
        std::future<ReturnType> QueueFunction(Func&& F, Args&&... args) noexcept;

        template <typename Func, typename... Args, typename ReturnType = std::invoke_result_t<std::decay_t<Func>, size_t, size_t, std::decay_t<Args>...>>
        inline [[nodiscard]]
        GroupTasks<ReturnType> QueueAndPartitionLoop(const size_t EndIndex, const size_t PartitionCount, Func&& F, Args&&... args) noexcept;

        template <typename Func, typename... Args, typename ReturnType = std::invoke_result_t<std::decay_t<Func>, size_t, size_t, std::decay_t<Args>...>>
        inline [[nodiscard]]
        GroupTasks<ReturnType> QueueAndPartitionLoop(const size_t StartIndex, const size_t EndIndex, const size_t PartitionCount, Func&& F, Args&&... args) noexcept;

        inline
        void WaitForAllTasks(void) noexcept;

        inline
        void ResetThreads(const size_t ThreadCount = 0) noexcept;

    private:

        /*
            Private Member Functions
        */
        inline
        void CreateThreads(void) noexcept;

        inline
        void DestroyThreads(void) noexcept;

        inline [[nodiscard]]
        size_t ComputeThreadCount(const size_t ThreadCount) noexcept;

        inline
        void WorkerThread(void) noexcept;

        /*
            Variables
        */
        size_t                           m_AvailableThreads       = std::thread::hardware_concurrency();
        std::atomic_bool                 m_Running                = false;
        std::atomic_bool                 m_Waiting                = false;
        std::atomic_bool                 m_Paused                 = false;
        std::atomic_int32_t              m_TotalTaskCount         = 0;
        std::mutex                       m_MutexLock              = {};
        std::condition_variable          m_CVNewTask              = {};
        std::condition_variable          m_CVTaskCompleted        = {};
        std::queue<VoidFunc>             m_TaskQueue              = {};
        std::unique_ptr<std::thread[]>   m_Threads                = nullptr;
    };
}