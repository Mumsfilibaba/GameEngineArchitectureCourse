#pragma once
#include <cstdlib>
#include <vector>
#include <mutex>
#include "SpinLock.h"

#include "MemoryManager.h"

template<typename T>
class PoolAllocator
{
private:
    struct Block
    {
        Block* pNext = nullptr;
    };


	class Chunk
	{
	public:
		inline Chunk(int sizeInBytes)
			: m_pMemory(nullptr),
			m_SizeInBytes(sizeInBytes)
		{
			constexpr size_t blockSize = std::max(sizeof(T), sizeof(Block));
			assert(sizeInBytes % blockSize == 0);

			//Allocate mem
			m_pMemory = malloc(m_SizeInBytes);
			
			//Init blocks
			Block* pOld = nullptr;
			Block* pCurrent = (Block*)m_pMemory;

			int blockCount = m_SizeInBytes / blockSize;
			for (int i = 0; i < blockCount; i++)
			{
				pCurrent->pNext = (Block*)(((char*)pCurrent) + blockSize); //HACKING;

				pOld = pCurrent;
				pCurrent = pCurrent->pNext;
			}

			//Set the last valid ptr's next to null
			pOld->pNext = nullptr;
		}


		inline ~Chunk()
		{
			if (m_pMemory)
			{
				free(m_pMemory);
				m_pMemory = nullptr;
			}
		}


		inline Block* GetFirstBlock() const
		{
			return (Block*)m_pMemory;
		}
	private:
		void* m_pMemory;
		size_t m_SizeInBytes;
	};
public:
    inline PoolAllocator(int chunkSizeInBytes = 4096)
        : m_ppChunks(),
		m_pFreeListHead(nullptr),
		m_pToFreeListHead(nullptr),
        m_ChunkSizeInBytes(chunkSizeInBytes),
		m_FreeLock(),
		m_ToFreeLock()
    {
		//Allocate chunk
		AllocateChunkAndSetHead();
    }

    
    inline ~PoolAllocator()
    {
		for (auto& pChunk : m_ppChunks)
		{
			delete pChunk;
			pChunk = nullptr;
		}
		m_ppChunks.clear();
    }


	inline void AllocateChunkAndSetHead()
	{
		Chunk* pChunk = new Chunk(m_ChunkSizeInBytes);
		m_ppChunks.emplace_back(pChunk);

		m_pFreeListHead = pChunk->GetFirstBlock();
	}


    inline void* AllocateBlock()
    {
		std::lock_guard<SpinLock> lock(m_FreeLock);
        
		Block* pCurrent = m_pFreeListHead;
		if (!pCurrent)
		{
			std::lock_guard<SpinLock> lock(m_ToFreeLock);

			m_pFreeListHead = m_pToFreeListHead;
			m_pToFreeListHead = nullptr;
			pCurrent = m_pFreeListHead;
			if (!pCurrent)
			{
				AllocateChunkAndSetHead();
				pCurrent = m_pFreeListHead;
			}
		}
		
		m_pFreeListHead = m_pFreeListHead->pNext;
        return (void*)pCurrent;
    }


    template<typename... Args>
    inline T* MakeNew(Args&& ... args)
    {
        return new(AllocateBlock()) T(std::forward<Args>(args) ...);
    }
    
    
    inline void Free(T* pObject)
    {
		std::lock_guard<SpinLock> lock(m_ToFreeLock);

		pObject->~T();

        Block* pFirst = (Block*)pObject;
        pFirst->pNext = m_pToFreeListHead;
		m_pToFreeListHead = pFirst;
    }
    
    
    inline int GetBlockSize() const
    {
        return sizeof(T);
    }
    
    
    inline int GetChunkSize() const
    {
        return m_ChunkSizeInBytes;
    }
private:
	std::vector<Chunk*> m_ppChunks;
    Block* m_pFreeListHead;
	Block* m_pToFreeListHead;
    size_t m_ChunkSizeInBytes;
	SpinLock m_FreeLock;
	SpinLock m_ToFreeLock;
};