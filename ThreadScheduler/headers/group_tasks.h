#pragma once

/*
Using Concepts On Non-Templated Member Functions Within Templated Class
- https://stackoverflow.com/questions/63338629/requires-clause-for-a-non-template-member-function-in-a-template-class
*/

namespace jpd
{
    template <typename ReturnType>
    class [[nodiscard]] GroupTasks final
    {
    public:

        explicit GroupTasks(const size_t Size) noexcept;

        inline void InsertFuture(std::future<ReturnType> Task) noexcept;

        inline [[nodiscard]]
        std::future<ReturnType>& GetFuture(const size_t Index) noexcept;

        inline [[nodiscard]]
        std::future<ReturnType>& operator[](const size_t Index) noexcept;

        void GetResults(void) noexcept requires(std::same_as<ReturnType, void>);

        inline [[nodiscard]]
        std::vector<ReturnType> GetResults(void) noexcept requires(!std::same_as<ReturnType, void>);

        inline void WaitForAll() noexcept;

    private:

        std::vector<std::future<ReturnType>> m_Tasks;
    };
}