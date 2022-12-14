#pragma once

namespace jpd
{
    PartitionTasks::PartitionTasks(const size_t PartitionCount, const size_t StartIndex, const size_t EndIndex) noexcept :
        m_PartitionCount{ PartitionCount }
    ,   m_StartIndex{StartIndex}
    ,   m_EndIndex{EndIndex}
    {
        assert(m_PartitionCount > 0);

        if (m_StartIndex > m_EndIndex)
        {
            std::swap(m_StartIndex, m_EndIndex);
        }
    }

    inline [[nodiscard]]
    std::vector<size_t> PartitionTasks::PartitionData(void) noexcept
    {
        return PartitionData(m_EndIndex - m_StartIndex);
    }

    template <typename Container>
    requires( std::ranges::contiguous_range<Container> )
    inline [[nodiscard]]
    std::vector<size_t> PartitionTasks::PartitionData(const Container& Data) noexcept
    {
        return PartitionData(Data.size());
    }

    inline [[nodiscard]]
        std::vector<size_t> PartitionTasks::PartitionData(const size_t DataCount) noexcept
    {
        size_t Block = (DataCount / m_PartitionCount);
        float  FBlock = (DataCount / static_cast<float>(m_PartitionCount));

        size_t MainBlock = Block != FBlock ? Block + 1
            : Block;
        size_t LastBlock = DataCount - (MainBlock * (m_PartitionCount - 1));

        std::vector<size_t> PartitionedGroupSize(m_PartitionCount, MainBlock);
        PartitionedGroupSize[PartitionedGroupSize.size() - 1] = LastBlock;

        // Returns The Size Of Each Partitioned Data Set
        return PartitionedGroupSize;
    }

    inline [[nodiscard]]
    std::vector<size_t> PartitionTasks::PartitionLoopIndices(void) noexcept
    {
        return PartitionLoopIndices(m_StartIndex, m_EndIndex);
    }

    inline [[nodiscard]]
    std::vector<size_t> PartitionTasks::PartitionLoopIndices(size_t StartIndex, size_t EndIndex) noexcept
    {
        if (EndIndex < StartIndex)
        {
            std::swap(StartIndex, EndIndex);
        }

        size_t DataCount = EndIndex - StartIndex;
        size_t Block = (DataCount / m_PartitionCount);
        float  FBlock = (DataCount / static_cast<float>(m_PartitionCount));

        size_t MainBlock = Block != FBlock ? Block + 1
                                           : Block;
        size_t LastBlock = DataCount - (MainBlock * (m_PartitionCount - 1));

        std::vector<size_t> PartitionedGroupSize(m_PartitionCount);

        for (size_t i = 0, max = PartitionedGroupSize.size(); i < max - 1; ++i)
        {
            PartitionedGroupSize[i] = StartIndex + i * MainBlock;
        }
        PartitionedGroupSize[PartitionedGroupSize.size() - 1] = EndIndex - LastBlock;

        // Returns A Vector Of Starting Indices Of The For Loop
        return PartitionedGroupSize;
    }
}