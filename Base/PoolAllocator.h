#pragma once
#include <cstdlib>
#include <vector>
#include <mutex>
#include <type_traits>
#include <algorithm>
#include "SpinLock.h"
#include "MemoryManager.h"
#include "Helpers.h"

#ifdef CONFIG_CHUNK_SIZE_4096
	#define CONFIG_CHUNK_SIZE 4096
#elif defined(CONFIG_CHUNK_SIZE_8192)
	#define CONFIG_CHUNK_SIZE 8192
#elif defined(CONFIG_CHUNK_SIZE_16384)
	#define CONFIG_CHUNK_SIZE 16384
#endif

#ifndef CONFIG_CHUNK_SIZE
	#define CHUNK_SIZE 4096
#else
	#define CHUNK_SIZE CONFIG_CHUNK_SIZE
#endif

#define CHUNK_SIZE_BYTES CHUNK_SIZE - sizeof(Arena*)

class PoolAllocatorBase
{
public:
	static int GetTotalAvailableMemory()
	{
		return s_TotalAllocated;
	}

	static int GetTotalUsedMemory()
	{
		return s_TotalUsed;
	}
protected:
	static std::atomic_int32_t s_TotalAllocated;
	static std::atomic_int32_t s_TotalUsed;
};

template<typename T>
class PoolAllocator : public PoolAllocatorBase
{
public:
    class Arena;
	
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

			//ThreadSafePrintf("Created %p\n", this);
#ifndef COLLECT_PERFORMANCE_DATA
			PoolAllocatorBase::s_TotalAllocated += CHUNK_SIZE;
#endif
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
#ifndef COLLECT_PERFORMANCE_DATA
			PoolAllocatorBase::s_TotalAllocated -= CHUNK_SIZE;
#endif
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

	class Arena
	{
	public:
		inline Arena() :
			m_pFreeListHead(nullptr),
			m_pBlocksToBeFreedHead(nullptr)
		{
			AllocateChunkAndSetHead();	
		}

		inline ~Arena()
		{
		}

		inline void Push(Block* block)
		{
			std::lock_guard<SpinLock> lock(m_FreeLock);
			block->pNext = m_pBlocksToBeFreedHead;
			m_pBlocksToBeFreedHead = block;

#ifndef COLLECT_PERFORMANCE_DATA
			PoolAllocatorBase::s_TotalUsed -= sizeof(T);
#endif
		}

		inline bool AllocateChunkAndSetHead()
		{
			Chunk* pChunk = new(mm_allocate(sizeof(Chunk), sizeof(Chunk), "Pool Allocation Chunk")) Chunk();
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
				{
					std::lock_guard<SpinLock> lock(m_FreeLock);
					m_pFreeListHead = m_pBlocksToBeFreedHead;
					m_pBlocksToBeFreedHead = nullptr;
				}

				pCurrent = m_pFreeListHead;
				if (!pCurrent)
				{
					AllocateChunkAndSetHead();
					pCurrent = m_pFreeListHead;
				}
			}

#ifndef COLLECT_PERFORMANCE_DATA
			PoolAllocatorBase::s_TotalUsed += sizeof(T);
#endif
			m_pFreeListHead = m_pFreeListHead->pNext;
			return pCurrent;
		}
	private:
		std::vector<Chunk*> m_Chunks;
		Block* m_pFreeListHead;
		Block* m_pBlocksToBeFreedHead;
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

	inline Arena* GetArena()
	{
		if (m_CurrentThread == nullptr)
		{
			m_CurrentThread = std::make_unique<Arena>();
		}
		return m_CurrentThread.get();
	}

#ifdef SHOW_ALLOCATIONS_DEBUG
	inline void* AllocateBlock(const std::string& tag)
	{
		constexpr size_t blockSize = std::max(sizeof(T), sizeof(Block));

		Block* block = GetArena()->Pop();

		MemoryManager::GetInstance().RegisterPoolAllocation(tag, (size_t)block, blockSize);
		return (void*)block;
	}
#else
	inline void* AllocateBlock()
	{
		Block* block = GetArena()->Pop();
		return (void*)block;
	}
#endif

	inline void Free(T* pObject)
	{
		pObject->~T(); //Call destructor
		FreeBlock(pObject);
	}

	inline void FreeBlock(void* ptr)
	{
		Block* block = (Block*)ptr;
		Chunk* chunk = Chunk::FromBlock(block);

#ifdef SHOW_ALLOCATIONS_DEBUG
		MemoryManager::GetInstance().RemovePoolAllocation((size_t)block);
#endif

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
	inline static thread_local std::unique_ptr<Arena> m_CurrentThread;
};

#ifdef SHOW_ALLOCATIONS_DEBUG
	#define pool_new(type, tag)		new(PoolAllocator<type>::Get().AllocateBlock(tag))
#else
	#define pool_new(type)			new(PoolAllocator<type>::Get().AllocateBlock())
#endif

#define pool_delete(object)		PoolAllocator< std::remove_pointer< std::remove_reference<decltype(object)>::type >::type >::Get().Free(object)
