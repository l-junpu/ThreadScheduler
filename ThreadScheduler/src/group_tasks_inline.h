#pragma once

namespace jpd
{
    template <typename ReturnType>
    GroupTasks<ReturnType>::GroupTasks(const size_t Size) noexcept :
        m_Tasks(Size)
    { }

    template <typename ReturnType>
    void GroupTasks<ReturnType>::InsertFuture(std::future<ReturnType> Task) noexcept
    {
        m_Tasks.push_back(std::move(Task));
    }

    template <typename ReturnType>
    [[nodiscard]]
    std::future<ReturnType>& GroupTasks<ReturnType>::GetFuture(const size_t Index) noexcept
    {
        assert(Index < m_Tasks.size());

        return m_Tasks[Index];
    }

    template <typename ReturnType>
    [[nodiscard]]
    std::future<ReturnType>& GroupTasks<ReturnType>::operator[](const size_t Index) noexcept
    {
        return GetFuture(Index);
    }

    template <typename ReturnType>
    void GroupTasks<ReturnType>::GetResults(void) noexcept requires(std::same_as<ReturnType, void>)
    {
        for (auto& Task : m_Tasks)
        {
            Task.get();
        }
        return;
    }

    template <typename ReturnType>
    [[nodiscard]]
    std::vector<ReturnType> GroupTasks<ReturnType>::GetResults(void) noexcept requires(!std::same_as<ReturnType, void>)
    {
        std::vector<ReturnType> Results(m_Tasks.size());
        for (size_t i = 0, max = m_Tasks.size(); i < max; ++i)
        {
            Results[i] = m_Tasks[i].get();
        }
        return Results;
    }

    template <typename ReturnType>
    void GroupTasks<ReturnType>::WaitForAll() noexcept
    {
        for (auto& Task : m_Tasks)
        {
            Task.wait();
        }
    }
}