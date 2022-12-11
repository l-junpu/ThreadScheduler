#pragma once

namespace jpd
{
    PartitionTasks::PartitionTasks(const size_t PartitionCount) noexcept : m_PartitionCount{ PartitionCount }
    {
        assert(m_PartitionCount > 0);
    }

    // Specialize This To Cover For C-Style Arrays
    template <typename Container>
    requires( std::ranges::contiguous_range<Container> )
    [[nodiscard]]
    std::vector<size_t> PartitionTasks::PartitionData(const Container& Data) noexcept
    {
        return PartitionData(Data.size());
    }

    [[nodiscard]]
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

    [[nodiscard]]
    std::vector<size_t> PartitionTasks::PartitionLoopIndices(size_t StartIndex, size_t EndIndex) noexcept
    {
        if (EndIndex < StartIndex) std::swap(StartIndex, EndIndex);

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

        for (auto i : PartitionedGroupSize)
            std::cout << "Split Loop Start Index: " << i << std::endl;

        // Returns A Vector Of Starting Indices Of The For Loop
        return PartitionedGroupSize;
    }
}