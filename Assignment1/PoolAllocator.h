#pragma once
#include <cstdlib>
#include <vector>
#include <mutex>
#include "SpinLock.h"
#include "MemoryManager.h"

#define MB(mb) mb * 1024 * 1024

template<typename T>
class PoolAllocator
{
public:
	struct Arena;
	
	//Node
    struct Block
    {
        Block* pNext = nullptr;
    };

	//Page
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
			m_pMemory = MemoryManager::GetInstance().Allocate(sizeInBytes, sizeInBytes, "Pool Allocation");
			
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
				//free(m_pMemory);
				m_pMemory = nullptr;
			}
		}


		inline Block* GetFirstBlock() const
		{
			return (Block*)m_pMemory;
		}
	public:
		void* m_pMemory;
		size_t m_SizeInBytes;
		Arena* m_pArena;
	};

	struct Arena
	{
		inline Arena() :
			m_pFreeListHead(nullptr),
			m_pToFreeListHead(nullptr)
		{

		}

		inline ~Arena()
		{
			for (auto& pChunk : m_Chunks)
			{
				delete pChunk;
				pChunk = nullptr;
			}
			m_Chunks.clear();
		}

		inline void push(Block* block)
		{
			std::lock_guard<SpinLock> lock(m_FreeLock);
			block->pNext = m_pToFreeListHead;
			m_pToFreeListHead = block;
		}

		inline bool AllocateChunkAndSetHead(size_t chunkSizeInBytes)
		{
			Chunk* chunk = new Chunk(chunkSizeInBytes);
			chunk->m_pArena = this;
			m_Chunks.emplace_back(chunk);
			m_pFreeListHead = chunk->GetFirstBlock();
			return true;
		}

		inline Block* pop(size_t chunkSizeInBytes)
		{
			Block* pCurrent = m_pFreeListHead;
			if (!pCurrent)
			{
				m_pFreeListHead = m_pToFreeListHead;
				m_pToFreeListHead = nullptr;
				pCurrent = m_pFreeListHead;
				if (!pCurrent)
				{
					AllocateChunkAndSetHead(chunkSizeInBytes);
					pCurrent = m_pFreeListHead;
				}
			}

			m_pFreeListHead = m_pFreeListHead->pNext;
			return pCurrent;
		}

	private:
		std::vector<Chunk*> m_Chunks;
		Block* m_pFreeListHead;
		Block* m_pToFreeListHead;
		SpinLock m_FreeLock;
	};

public:
    inline PoolAllocator(int chunkSizeInBytes = 4096)
        : m_ChunkSizeInBytes(chunkSizeInBytes)
    {

    }
    
    inline ~PoolAllocator()
    {
		
    }


    template<typename... Args>
    inline T* MakeNew(Args&& ... args)
    {
        return new(AllocateBlock()) T(std::forward<Args>(args) ...);
    }
    
    
    inline int GetBlockSize() const
    {
        return sizeof(T);
    }
    
    
    inline int GetChunkSize() const
    {
        return m_ChunkSizeInBytes;
    }

    
	inline int GetTotalMemory() const
	{
		return 5;
		//return m_ChunkSizeInBytes * m_ppChunks.size();
	}

	inline Arena* getArena()
	{
		Arena* arena = m_Current_thread.get();
		if (!arena)
		{
			arena = new Arena();
			m_Current_thread.reset(arena);
		}
		return arena;
	}

	inline void* AllocateBlock()
	{
		Block* block = getArena()->pop(m_ChunkSizeInBytes);
		return (void*)block;
	}

	inline void Free(T* pObject)
	{
		pObject->~T();
		Block* pFirst = (Block*)pObject;
		getArena()->push(pFirst);
	}

private:
    size_t m_ChunkSizeInBytes;
	inline thread_local static std::unique_ptr<Arena> m_Current_thread;
};
