#include "MemoryManager.h"
#include <thread>

std::atomic_int32_t MemoryManager::s_TotalAllocated = 0;
std::atomic_int32_t MemoryManager::s_TotalUsed = 0;

MemoryManager::MemoryManager()
	: m_pMemory(malloc(SIZE_IN_BYTES))
{
	m_pFreeTail = new(m_pMemory) FreeEntry(SIZE_IN_BYTES / 2);
	m_pFreeHead = new((void*)((size_t)m_pMemory + SIZE_IN_BYTES / 2)) FreeEntry(SIZE_IN_BYTES / 2);
	m_pFreeTail->pNext = m_pFreeHead;
	m_pFreeHead->pNext = m_pFreeTail;
	
	m_pFreeListStart = m_pFreeTail;

#ifndef COLLECT_PERFORMANCE_DATA
	s_TotalAllocated = SIZE_IN_BYTES;
#endif
}

MemoryManager::~MemoryManager()
{
	if (m_pMemory != nullptr)
	{
		free(m_pMemory);
		m_pMemory = nullptr;
	}

	m_pFreeListStart = nullptr;
	m_pFreeHead = nullptr;
	m_pFreeTail = nullptr;
}

void MemoryManager::Reset()
{
	if (m_pMemory != nullptr)
	{
		free(m_pMemory);
		m_pMemory = nullptr;
	}

	m_pFreeListStart = nullptr;
	m_pFreeHead = nullptr;
	m_pFreeTail = nullptr;

	m_pMemory = malloc(SIZE_IN_BYTES);
	m_pFreeTail = new(m_pMemory) FreeEntry(SIZE_IN_BYTES / 2);
	m_pFreeHead = new((void*)((size_t)m_pMemory + SIZE_IN_BYTES / 2)) FreeEntry(SIZE_IN_BYTES / 2);
	m_pFreeTail->pNext = m_pFreeHead;
	m_pFreeHead->pNext = m_pFreeTail;

	m_pFreeListStart = m_pFreeTail;
}

void* MemoryManager::Allocate(size_t allocationSizeInBytes, size_t alignment, const std::string& tag)
{
	assert(alignment % 2 == 0 || alignment == 1);

	std::lock_guard<SpinLock> lock(m_MemoryLock);

	FreeEntry* pLastFree = m_pFreeTail;
	FreeEntry* pCurrentFree = m_pFreeHead;

	do
	{
		size_t offset = (size_t)pCurrentFree;
		size_t padding = (-int64_t(offset) & (alignment - 1));
		size_t aligned = offset + padding;

		//Not Perfect Fit Free Block (Need to create a new FreeEntry after the allocation)
		if (pCurrentFree->sizeInBytes >= allocationSizeInBytes + padding)
		{
			FreeEntry* pCurrentFreeNext = pCurrentFree->pNext;

			size_t internalPadding = padding;
			size_t externalPadding = 0;

			FreeEntry* pNewFreeEntryBefore = nullptr;
			FreeEntry* pNewFreeEntryAfter = nullptr;

			if (pCurrentFree->sizeInBytes >= allocationSizeInBytes + padding + sizeof(FreeEntry))
			{
				//Create new FreeEntry after the allocation
				pNewFreeEntryAfter = new((void*)(aligned + allocationSizeInBytes)) FreeEntry(pCurrentFree->sizeInBytes - (allocationSizeInBytes + padding));
			}

			//Check if Padding can fit a Free Block
			if (padding > sizeof(FreeEntry))
			{
				pNewFreeEntryBefore = new(pCurrentFree) FreeEntry(padding);

				externalPadding = internalPadding;
				internalPadding = 0;
			}

			if (pNewFreeEntryBefore != nullptr && pNewFreeEntryAfter != nullptr)
			{
				pLastFree->pNext = pNewFreeEntryBefore;
				pNewFreeEntryBefore->pNext = pNewFreeEntryAfter;
				pNewFreeEntryAfter->pNext = pCurrentFreeNext;

				m_pFreeTail = pNewFreeEntryBefore;
				m_pFreeHead = pNewFreeEntryAfter;
			}
			else if (pNewFreeEntryBefore != nullptr)
			{
				pLastFree->pNext = pNewFreeEntryBefore;
				pNewFreeEntryBefore->pNext = pCurrentFreeNext;

				m_pFreeTail = pNewFreeEntryBefore;
				m_pFreeHead = pCurrentFreeNext;
			}
			else if (pNewFreeEntryAfter != nullptr)
			{
				pLastFree->pNext = pNewFreeEntryAfter;
				pNewFreeEntryAfter->pNext = pCurrentFreeNext;

				m_pFreeTail = pLastFree;
				m_pFreeHead = pNewFreeEntryAfter;
			}

			//Create a new Allocation at the CurrentFree Address
			m_AllocationHeaders[aligned] = Allocation(tag, allocationSizeInBytes + internalPadding, internalPadding);

			//Return the address of the allocation, but offset it so the size member does not get overridden.
#ifndef COLLECT_PERFORMANCE_DATA
			s_TotalUsed += (allocationSizeInBytes + padding);
#endif
			return (void*)(aligned);
		}

		pLastFree = pCurrentFree;
		pCurrentFree = pCurrentFree->pNext;

	} while (pCurrentFree != m_pFreeHead);

	return nullptr;
}

void MemoryManager::Free(void* allocationPtr)
{
	std::lock_guard<SpinLock> lock(m_MemoryLock);

	size_t allocationAddress = (size_t)allocationPtr;
	Allocation& allocation = m_AllocationHeaders[allocationAddress];
	m_AllocationHeaders.erase(allocationAddress);

	size_t offsetAllocationAddress = allocationAddress - allocation.padding;

	FreeEntry* pLastFree = m_pFreeTail;
	FreeEntry* pCurrentFree = m_pFreeHead;

	do
	{
		//We encounter a block that ends in our start (Coalesce left)
		if ((size_t)pCurrentFree + pCurrentFree->sizeInBytes == offsetAllocationAddress)
		{
			FreeEntry* pNewFreeEntry = nullptr;

			//We have a block on our right that we can coalesce with as well
			if (offsetAllocationAddress + allocation.sizeInBytes == (size_t)pCurrentFree->pNext)
			{
				size_t newFreeSize = pCurrentFree->sizeInBytes + allocation.sizeInBytes + pCurrentFree->pNext->sizeInBytes;

				FreeEntry* pCurrentFreeNextNext = pCurrentFree->pNext->pNext;
				pNewFreeEntry = new(pCurrentFree) FreeEntry(newFreeSize);
				pLastFree->pNext = pNewFreeEntry;
				pNewFreeEntry->pNext = pCurrentFreeNextNext;
			}
			else
			{
				size_t newFreeSize = pCurrentFree->sizeInBytes + allocation.sizeInBytes;

				FreeEntry* pCurrentFreeNext = pCurrentFree->pNext;
				pNewFreeEntry = new(pCurrentFree) FreeEntry(newFreeSize);
				pLastFree->pNext = pNewFreeEntry;
				pNewFreeEntry->pNext = pCurrentFreeNext;
			}

			m_pFreeHead = pNewFreeEntry;
			m_pFreeTail = pLastFree;

			return;
		}
		//We encounter a block that starts in our end (Coalesce right)
		else if (offsetAllocationAddress + allocation.sizeInBytes == (size_t)pCurrentFree)
		{
			size_t newFreeSize = allocation.sizeInBytes + pCurrentFree->sizeInBytes;

			FreeEntry* pCurrentFreeNext = pCurrentFree->pNext;
			FreeEntry* pNewFreeEntry = new(pCurrentFree) FreeEntry(newFreeSize);
			pLastFree->pNext = pNewFreeEntry;
			pNewFreeEntry->pNext = pCurrentFreeNext;

			m_pFreeHead = pNewFreeEntry;
			m_pFreeTail = pLastFree;

			return;
		}

		pLastFree = pCurrentFree;
		pCurrentFree = pCurrentFree->pNext;

	} while (pCurrentFree != m_pFreeHead);
}

#ifdef SHOW_ALLOCATIONS_DEBUG
void MemoryManager::RegisterPoolAllocation(const std::string& tag, size_t startAddress, size_t size)
{
	std::lock_guard<SpinLock> lock(m_PoolAllocationLock);
	m_PoolAllocations[startAddress] = SubAllocation(tag, size);
}

void MemoryManager::RemovePoolAllocation(size_t startAddress)
{
	std::lock_guard<SpinLock> lock(m_PoolAllocationLock);
	m_PoolAllocations.erase(startAddress);
}

void MemoryManager::RegisterStackAllocation(const std::string& tag, size_t startAddress, size_t size)
{
	std::lock_guard<SpinLock> lock(m_StackAllocationLock);
	m_StackAllocations[startAddress] = SubAllocation(tag, size);
}

void MemoryManager::ClearStackAllocations()
{
	std::lock_guard<SpinLock> lock(m_StackAllocationLock);
	m_StackAllocations.clear();
}
#endif
