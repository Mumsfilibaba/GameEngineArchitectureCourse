#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include <cstdlib>
#include <cassert>
#include <mutex>

//#define SIZE_IN_BYTES (64 * 1024)
#define SIZE_IN_BYTES 1024 * 1024 * 1024

class MemoryManager
{
	struct Allocation
	{
		Allocation(size_t sizeInBytes)
		{
			this->sizeInBytes = sizeInBytes;
		}

		size_t sizeInBytes = 0;
	};

	struct FreeEntry
	{
		FreeEntry(size_t sizeInBytes, FreeEntry* pNext)
		{
			this->sizeInBytes = sizeInBytes;
			this->pNext = pNext;
		}

		size_t sizeInBytes = 0;
		FreeEntry* pNext = nullptr;
	};

public:
	~MemoryManager();
	void* Allocate(size_t sizeInBytes);
	void Free(void* allocation);

	MemoryManager(MemoryManager const&) = delete;
	void operator=(MemoryManager const&) = delete;

private:
	MemoryManager();

private:
	void* m_pMemory;
	FreeEntry* m_pFreeList;

public:
	static MemoryManager& getInstance()
	{
		static MemoryManager instance;
		return instance;
	}
};

#endif