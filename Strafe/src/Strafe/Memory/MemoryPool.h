
#pragma once

namespace strafe
{
	template<size_t sz>
	struct MemoryBlock
	{
		static_assert(sz > sizeof(uint64_t));
		union
		{
			MemoryBlock* m_Next{ nullptr };	//if unused, it will be used as a pointer to the next block
			struct
			{
				uint64_t m_ContiguousBlocksInUse;	//if used, it will point to how many blocks in use
				uint8_t m_Payload[sz - sizeof(uint64_t)];			//data
			} m_Data;
		};
	};

	template<size_t blockSize, size_t blockCount>
	struct MemoryPool
	{
		MemoryBlock<blockSize>* m_MemoryBlocks{ nullptr };
		MemoryBlock<blockSize>* m_FreeBlock{ nullptr };

		MemoryPool()
		{
			m_MemoryBlocks = new MemoryBlock<blockSize>[blockCount];
			m_FreeBlock = m_MemoryBlocks;
			for (size_t i = 0; i < blockCount - 1; i++)
			{
				m_MemoryBlocks[i].m_Next = &m_MemoryBlocks[i + 1];
				//RD_CORE_TRACE("{}", reinterpret_cast<uint8_t*>(m_MemoryBlocks[i].m_Next) - reinterpret_cast<uint8_t*>(&m_MemoryBlocks[i]));
			}
		}

		~MemoryPool()
		{
			delete[] m_MemoryBlocks;
		}

		bool CheckValid(void* p) const
		{
			if (!p)
				return false;
			int64_t offset = static_cast<uint8_t*>(p) - reinterpret_cast<uint8_t*>(m_MemoryBlocks);
			if (offset < 0 || offset >= blockSize * blockCount)
				return false;
			offset -= sizeof(uint64_t);
			if (offset % blockSize == 0)
				return true;
			return false;
		}
	};
}
