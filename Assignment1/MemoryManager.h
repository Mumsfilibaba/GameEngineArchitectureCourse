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
#define allocate(...) MemoryManager::GetInstance().Allocate(__VA_ARGS__)

struct Allocation
{
	Allocation(size_t sizeInBytes, size_t padding)
	{
		this->sizeInBytes = sizeInBytes;
		this->padding = padding;
	}

	size_t sizeInBytes = 0;
	size_t padding = 0;
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
	void* Allocate(size_t sizeInBytes, size_t alignment, const std::string& tag);
	void Free(void* allocation);
	const std::map<size_t, std::string>& GetAllocations() { return m_Allocations; }
	const FreeEntry* GetFreeList() { return m_pFreeListStart; };

	MemoryManager(MemoryManager const&) = delete;
	void operator=(MemoryManager const&) = delete;

private:
	MemoryManager();
	void RegisterAllocation(
		const std::string& tag,
		size_t startAddress,
		size_t allocationAddress,
		size_t returnedMemoryAddress,
		size_t endAddress,
		size_t blockSizeInBytes,
		size_t allocationSizeInBytes,
		size_t alignment,
		size_t internalPadding,
		size_t externalPadding,
		size_t extraMemory);
	void RemoveAllocation(size_t address);
	template <typename I> std::string N2HexStr(I w, size_t hex_len = sizeof(I) << 1);

private:
	void* m_pMemory;
	FreeEntry* m_pFreeListStart;
	FreeEntry* m_pFreeHead;
	FreeEntry* m_pFreeTail;
	std::map<size_t, std::string> m_Allocations;
	SpinLock m_MemoryLock;

public:
	static MemoryManager& GetInstance()
	{
		static MemoryManager instance;
		return instance;
	}
};

template <typename I>
std::string MemoryManager::N2HexStr(I w, size_t hex_len)
{
	static const char* digits = "0123456789abcdef";
	std::string rc(hex_len + 2, '0');
	rc[0] = '0';
	rc[1] = 'x';
	for (size_t i = 2, j = (hex_len - 1) * 4; i < hex_len + 2; ++i, j -= 4)
		rc[i] = digits[(w >> j) & 0x0f];
	return rc;
}

#endif
