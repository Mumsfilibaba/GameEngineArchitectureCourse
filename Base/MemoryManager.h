#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include <cstdlib>
#include <cassert>
#include <mutex>
#include <map>
#include <iomanip>
#include <string>
#include <sstream>
#include <unordered_map>
#include "SpinLock.h"
#include "Helpers.h"

//#define SIZE_IN_BYTES (64 * 1024)
#define SIZE_IN_BYTES 1024 * 1024 * 512 // = 128mb
#define mm_allocate(...) MemoryManager::GetInstance().Allocate(__VA_ARGS__)
#define mm_free(...) MemoryManager::GetInstance().Free(__VA_ARGS__)

struct Allocation
{
	Allocation()
	{
		this->tag = "";
		this->sizeInBytes = 0;
		this->padding = 0;
	}

	Allocation(const std::string& tag, size_t sizeInBytes, size_t padding)
	{
		this->tag = tag;
		this->sizeInBytes = sizeInBytes;
		this->padding = padding;
	}

	std::string tag;
	size_t sizeInBytes;
	size_t padding;
};

struct SubAllocation
{
	SubAllocation()
	{
		this->tag = "";
		this->sizeInBytes = 0;
	}

	SubAllocation(const std::string& tag, size_t sizeInBytes)
	{
		this->tag = tag;
		this->sizeInBytes = sizeInBytes;
	}

	std::string tag;
	size_t sizeInBytes;
};

struct FreeEntry
{
	FreeEntry(size_t sizeInBytes)
	{
		this->sizeInBytes = sizeInBytes;
	}

	size_t sizeInBytes = 0;
	FreeEntry* pNext = nullptr;
};

struct DebugFreeEntry
{
	DebugFreeEntry()
	{
		this->sizeInBytes = 0;
		this->nextAddress = 0;
	}

	DebugFreeEntry(const FreeEntry* pFreeEntry)
	{
		this->sizeInBytes = pFreeEntry->sizeInBytes;
		this->nextAddress = (size_t)pFreeEntry->pNext;
	}

	size_t sizeInBytes = 0;
	size_t nextAddress = 0;
};

class MemoryManager
{
public:
	~MemoryManager();
	void* Allocate(size_t allocationSizeInBytes, size_t alignment, const std::string& tag);
	void Free(void* allocation);

	void RegisterPoolAllocation(const std::string& tag, size_t startAddress, size_t size);
	void RemovePoolAllocation(size_t startAddress);
	const std::map<size_t, SubAllocation>& GetPoolAllocations() { return m_PoolAllocations; }
	
#ifdef SHOW_ALLOCATIONS_DEBUG
	void RegisterStackAllocation(const std::string& tag, size_t startAddress, size_t size);
	void ClearStackAllocations();
	const std::map<size_t, SubAllocation>& GetStackAllocations() { return m_StackAllocations; }

	const std::map<size_t, Allocation>& GetAllocations() { return m_AllocationHeaders; }
	const void* GetMemoryStart() { return m_pMemory; }
	const FreeEntry* GetFreeListHead() { return m_pFreeHead; };
#endif

	SpinLock& GetMemoryLock() { return m_MemoryLock; }
	SpinLock& GetPoolAllocationLock() { return m_PoolAllocationLock; }
	SpinLock& GetStackAllocationLock() { return m_StackAllocationLock; }

	MemoryManager(MemoryManager const&) = delete;
	void operator=(MemoryManager const&) = delete;

private:
	MemoryManager();
	void PrintFreeList();
	void CheckFreeListCorruption();

private:
	void* m_pMemory;
	std::map<size_t, Allocation> m_AllocationHeaders;
	FreeEntry* m_pFreeHead;
	FreeEntry* m_pFreeTail;
	SpinLock m_MemoryLock;

	std::map<size_t, SubAllocation> m_PoolAllocations;
	std::map<size_t, SubAllocation> m_StackAllocations;
	SpinLock m_PoolAllocationLock;
	SpinLock m_StackAllocationLock;
public:
	static MemoryManager& GetInstance()
	{
		static MemoryManager instance;
		return instance;
	}

	static size_t GetTotalAvailableMemory()
	{
		return s_TotalAllocated;
	}

	static size_t GetTotalUsedMemory()
	{
		return s_TotalUsed;
	}
private:
	static std::atomic_size_t s_TotalAllocated;
	static std::atomic_size_t s_TotalUsed;
};

#endif
