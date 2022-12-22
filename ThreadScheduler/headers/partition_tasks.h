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

        explicit PartitionTasks( const size_t PartitionCount
                               , const size_t StartIndex = 0
                               , const size_t EndIndex   = 0 ) noexcept;

        inline [[nodiscard]]
        std::vector<size_t> PartitionData(void) noexcept;

        template <typename Container>
        requires( std::ranges::contiguous_range<Container> )
        inline [[nodiscard]]
        std::vector<size_t> PartitionData(const Container& Data) noexcept;

        inline [[nodiscard]]
        std::vector<size_t> PartitionData(const size_t DataCount) noexcept;

        inline [[nodiscard]]
        std::vector<size_t> PartitionLoopIndices(void) noexcept;

        static inline [[nodiscard]]
        std::vector<size_t> PartitionLoopIndices( size_t       StartIndex
                                                , size_t       EndIndex
                                                , const size_t PartitionCount
                                                , const size_t MinimumPartitionSize = 0 ) noexcept;

    private:

        size_t m_StartIndex     = 0;
        size_t m_EndIndex       = 0;
        size_t m_PartitionCount = 0;
    };
}