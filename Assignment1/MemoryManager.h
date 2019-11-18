#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include <cstdlib>
#include <cassert>
#include <mutex>
#include <map>
#include <iomanip>
#include <string>
#include <sstream>

#include "SpinLock.h"

//#define SIZE_IN_BYTES (64 * 1024)
#define SIZE_IN_BYTES 1024 * 1024 * 1024

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

class MemoryManager
{
public:
	~MemoryManager();
	void* Allocate(size_t sizeInBytes, const std::string& tag);
	void Free(void* allocation);
	const std::map<size_t, std::string>& GetAllocations() { return m_Allocations; }
	const FreeEntry* GetFreeList() { return m_pFreeList; };

	MemoryManager(MemoryManager const&) = delete;
	void operator=(MemoryManager const&) = delete;

private:
	MemoryManager();
	void RegisterAllocation(size_t sizeInBytes, const std::string& tag, size_t address);
	void RemoveAllocation(size_t address);

private:
	void* m_pMemory;
	FreeEntry* m_pFreeList;
	std::map<size_t, std::string> m_Allocations;
	SpinLock m_MemoryLock;

public:
	static MemoryManager& GetInstance()
	{
		static MemoryManager instance;
		return instance;
	}
};

#endif