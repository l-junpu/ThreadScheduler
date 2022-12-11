#pragma once

/*
Deduce Type Of Invoke Expressions At Compile Time
- https://en.cppreference.com/w/cpp/types/result_of
- Must Satisfy Conditions of "Callable"
    - https://en.cppreference.com/w/cpp/named_req/Callable
*/

namespace jpd
{
    class [[nodiscard]] PartitionTasks final
    {
    public:

        explicit PartitionTasks(const size_t PartitionCount) noexcept; // probably accept start and end indexes also

        // Specialize This To Cover For C-Style Arrays
        template <typename Container>
        requires( std::ranges::contiguous_range<Container> )
        inline [[nodiscard]]
        std::vector<size_t> PartitionData(const Container& Data) noexcept;

        inline [[nodiscard]]
        std::vector<size_t> PartitionData(const size_t DataCount) noexcept;

        inline [[nodiscard]]
        std::vector<size_t> PartitionLoopIndices(const size_t StartIndex, const size_t EndIndex) noexcept;

    private:

        size_t m_PartitionCount = 1;
    };
}