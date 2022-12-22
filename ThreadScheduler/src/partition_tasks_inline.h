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
        // There Should Be Elements To Partition
        assert(DataCount > 0);

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
        return PartitionTasks::PartitionLoopIndices(m_StartIndex, m_EndIndex, m_PartitionCount);
    }

    inline [[nodiscard]]
    std::vector<size_t> PartitionTasks::PartitionLoopIndices(size_t StartIndex, size_t EndIndex, const size_t PartitionCount, const size_t MinimumPartitionSize) noexcept
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
            std::cout << "Index: " << PartitionedGroupSize[i] << std::endl;
        }
        PartitionedGroupSize[PartitionedGroupSize.size() - 1] = EndIndex;
        std::cout << "Index: " << PartitionedGroupSize[PartitionedGroupSize.size() - 1] << std::endl;

        // Returns A Vector Of Starting Indices Of The For Loop
        return PartitionedGroupSize;
    }
}