#pragma once
#include <cstdlib>
#include <vector>
#include <mutex>
#include <type_traits>
#include "SpinLock.h"
#include "MemoryManager.h"

#define MB(mb) mb * 1024 * 1024
#define CHUNK_SIZE 4096
#define CHUNK_SIZE_BYTES CHUNK_SIZE - sizeof(Arena*)

template<typename T>
class PoolAllocator
{
public:
	struct Arena;
	
    struct Block
    {
        Block* pNext = nullptr;
    };

	class Chunk
	{
	public:
		inline Chunk()
		{
			constexpr size_t blockSize = std::max(sizeof(T), sizeof(Block));
			static_assert(CHUNK_SIZE % blockSize == 0);

			//ThreadSafePrintf("Created %p\n", this);

			//Allocate mem
			s_TotalMemoryUsed += CHUNK_SIZE;
			
			//Init blocks
			constexpr int chunkSize		= CHUNK_SIZE_BYTES;
			constexpr int blockCount	= chunkSize / blockSize;
			
			Block* pCurrent = (Block*)m_Memory;
			for (int i = 1; i < blockCount; i++)
			{
				pCurrent->pNext = (Block*)(((char*)pCurrent) + blockSize); //HACKING;
				pCurrent = pCurrent->pNext;
			}

			//Set the last valid ptr's next to null
			pCurrent->pNext = nullptr;
		}

		inline ~Chunk()
		{
			s_TotalMemoryUsed -= CHUNK_SIZE;
			m_pArena = nullptr;
		}

		inline Block* GetFirstBlock() const
		{
			return (Block*)m_Memory;
		}

		inline static Chunk* FromBlock(Block* block)
		{
			constexpr size_t mask = sizeof(Chunk) - 1;
			return reinterpret_cast<Chunk*>((size_t)block & ~(mask));
		}
	public:
		Arena* m_pArena;
		char m_Memory[CHUNK_SIZE_BYTES];
	};

	struct Arena
	{
		inline Arena() :
			m_pFreeListHead(nullptr),
			m_pToFreeListHead(nullptr)
		{
			AllocateChunkAndSetHead();	
		}

		inline ~Arena()
		{
			m_Chunks.clear();
		}

		inline void Push(Block* block)
		{
			std::lock_guard<SpinLock> lock(m_FreeLock);
			block->pNext = m_pToFreeListHead;
			m_pToFreeListHead = block;
		}

		inline bool AllocateChunkAndSetHead()
		{
			void* pMem = allocate(sizeof(Chunk), sizeof(Chunk), "Pool Allocation Chunk");
			Chunk* pChunk = new(pMem) Chunk();
			pChunk->m_pArena = this;

			m_Chunks.emplace_back(pChunk);
			m_pFreeListHead = pChunk->GetFirstBlock();
			return true;
		}

		inline Block* Pop()
		{
			Block* pCurrent = m_pFreeListHead;
			if (!pCurrent)
			{
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
			return pCurrent;
		}
	private:
		std::vector<Chunk*> m_Chunks;
		Block* m_pFreeListHead;
		Block* m_pToFreeListHead;
		SpinLock m_FreeLock;
	};
public:
    inline PoolAllocator()
    {
		static_assert(sizeof(T) < CHUNK_SIZE, "sizeof type is too big");
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
		return s_TotalMemoryUsed;
	}

	inline Arena* GetArena()
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
		Block* block = GetArena()->Pop();
		return (void*)block;
	}

	inline void Free(T* pObject)
	{
		Block* block = (Block*)pObject;
		Chunk* chunk = Chunk::FromBlock(block);

		pObject->~T();

		//ThreadSafePrintf("Recived %p\n", chunk);

		Arena* arena = chunk->m_pArena;
		assert(arena);
		arena->Push(block);
	}
public:
	static PoolAllocator& Get()
	{
		static PoolAllocator instance;
		return instance;
	}
private:
	inline static int s_TotalMemoryUsed = 0;
	inline thread_local static std::unique_ptr<Arena> m_Current_thread;
};

#define pool_new(type)			new(PoolAllocator<type>::Get().AllocateBlock())
#define pool_delete(object)		PoolAllocator< std::remove_pointer< std::remove_reference<decltype(object)>::type >::type >::Get().Free(object);
