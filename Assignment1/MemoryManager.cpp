#include "MemoryManager.h"

MemoryManager::MemoryManager()
	: m_pMemory(malloc(SIZE_IN_BYTES))
{
	m_pFreeList = new(m_pMemory) FreeEntry(SIZE_IN_BYTES, nullptr);
}

MemoryManager::~MemoryManager()
{
	if (m_pMemory != nullptr)
	{
		free(m_pMemory);
		m_pMemory = nullptr;
	}

	m_pFreeList = nullptr;
}

void* MemoryManager::Allocate(size_t sizeInBytes)
{
	size_t totalAllocationSize = sizeInBytes + sizeof(size_t);

	FreeEntry* pLastFree = nullptr;
	FreeEntry* pCurrentFree = m_pFreeList;

	while (pCurrentFree != nullptr)
	{
		//Not Perfect Fit Free Block (Need to create a new FreeEntry after the allocation)
		if (pCurrentFree->sizeInBytes >= totalAllocationSize + sizeof(FreeEntry))
		{
			//Create new FreeEntry after the allocation
			FreeEntry* pNewFreeEntry = new((void*)((size_t)pCurrentFree + totalAllocationSize)) FreeEntry(pCurrentFree->sizeInBytes - totalAllocationSize, pCurrentFree->pNext);

			//If last free is nullptr we are at start of FreeList
			if (pLastFree != nullptr)
				pLastFree->pNext = pNewFreeEntry;
			else
				m_pFreeList = pNewFreeEntry;

			//Create a new Allocation at the CurrentFree Address
			Allocation* pAllocation = new(pCurrentFree) Allocation(totalAllocationSize);

			//Return the address of the allocation, but offset it so the size member does not get overridden.
			return (void*)((size_t)pAllocation + sizeof(size_t));
		}
		//Perfect Fit Free Block (No need to create a new FreeEntry after the allocation)
		else if (pCurrentFree->sizeInBytes == totalAllocationSize)
		{
			//If last free is nullptr we are at start of FreeList
			if (pLastFree != nullptr)
				pLastFree->pNext = pCurrentFree->pNext;
			else
				m_pFreeList = pCurrentFree->pNext;

			//Create a new Allocation at the CurrentFree Address
			Allocation* pAllocation = new(pCurrentFree) Allocation(totalAllocationSize);

			//Return the address of the allocation, but offset it so the size member does not get overridden.
			return (void*)((size_t)pAllocation + sizeof(size_t));
		}

		pLastFree = pCurrentFree;
		pCurrentFree = pCurrentFree->pNext;
	}

	return nullptr;
}

void MemoryManager::Free(void* allocation)
{
	Allocation* pAllocation = reinterpret_cast<Allocation*>(((size_t)allocation - sizeof(size_t))); //HACKING
	size_t allocationAddress = (size_t)pAllocation;

	FreeEntry* pClosestLeft = nullptr;

	FreeEntry* pLastFree = nullptr;
	FreeEntry* pCurrentFree = m_pFreeList;

	while (pCurrentFree != nullptr)
	{
		if ((size_t)pAllocation + pAllocation->sizeInBytes == (size_t)pCurrentFree)
		{
			size_t newFreeSize = pAllocation->sizeInBytes + pCurrentFree->sizeInBytes;
			//Memset?

			if (pLastFree != nullptr)
				pLastFree->pNext = new(pAllocation) FreeEntry(newFreeSize, pCurrentFree->pNext);
			else
				m_pFreeList = new(pAllocation) FreeEntry(newFreeSize, pCurrentFree->pNext);
			return;
		}

		if (allocationAddress > (size_t)pLastFree && (size_t)pLastFree > (size_t)pClosestLeft)
			pClosestLeft = pLastFree;

		pLastFree = pCurrentFree;
		pCurrentFree = pCurrentFree->pNext;
	}

	if (pClosestLeft == nullptr)
		m_pFreeList = new(pAllocation) FreeEntry(pAllocation->sizeInBytes, m_pFreeList->pNext);
	else
		pClosestLeft->pNext = new(pAllocation) FreeEntry(pAllocation->sizeInBytes, pClosestLeft->pNext);
}
