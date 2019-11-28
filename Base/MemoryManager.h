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
#define SIZE_IN_BYTES 1024 * 1024 * 64 // = 64mb
#define allocate(...) MemoryManager::GetInstance().Allocate(__VA_ARGS__)
#define release(...) MemoryManager::GetInstance().Free(__VA_ARGS__)

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

class MemoryManager
{
public:
	~MemoryManager();
	void Reset();
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
	const FreeEntry* GetFreeList() { return m_pFreeListStart; };
#endif

	SpinLock& GetMemoryLock() { return m_MemoryLock; }
	SpinLock& GetPoolAllocationLock() { return m_PoolAllocationLock; }
	SpinLock& GetStackAllocationLock() { return m_StackAllocationLock; }

	MemoryManager(MemoryManager const&) = delete;
	void operator=(MemoryManager const&) = delete;

private:
	MemoryManager();

private:
	void* m_pMemory;
	std::map<size_t, Allocation> m_AllocationHeaders;
	FreeEntry* m_pFreeListStart;
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

	static int GetTotalAvailableMemory()
	{
		return s_TotalAllocated;
	}

	static int GetTotalUsedMemory()
	{
		return s_TotalUsed;
	}
private:
	static std::atomic_int32_t s_TotalAllocated;
	static std::atomic_int32_t s_TotalUsed;
};

#endif
