#include "MemoryManager.h"
#include "FrameAllocator.h"

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

	FrameAllocator::Release();
}

void MemoryManager::RegisterAllocation(size_t sizeInBytes, size_t padding, size_t address, const std::string& tag)
{
	size_t mb = sizeInBytes / (1024 * 1024);
	size_t kb = (sizeInBytes - mb * (1024 * 1024)) / 1024;
	size_t bytes = (sizeInBytes - mb * (1024 * 1024) - kb * 1024);

	std::stringstream stream;
	stream << std::hex << address;
	m_Allocations[address] = 
		"A" + tag + 
		"\nStart: " + stream.str() + 
		"\nSize: " + 
        "\nPadding:" + std::to_string(padding) +
		std::to_string(mb) + "MB " +
		std::to_string(kb) + "kB " +
		std::to_string(bytes) + "bytes";
}

void MemoryManager::RemoveAllocation(size_t address)
{
	m_Allocations.erase(address);
}

void* MemoryManager::Allocate(size_t sizeInBytes, size_t alignment, const std::string& tag)
{
	assert(alignment % 2 == 0);

	size_t totalAllocationSize = sizeInBytes + sizeof(Allocation);

	std::lock_guard<SpinLock> lock(m_MemoryLock);

	FreeEntry* pLastFree = nullptr;
	FreeEntry* pCurrentFree = m_pFreeList;

	while (pCurrentFree != nullptr)
	{
		size_t offset = (size_t)pCurrentFree;
		size_t padding = (-offset & (alignment - 1));
		size_t aligned = offset + padding;
		//Not Perfect Fit Free Block (Need to create a new FreeEntry after the allocation)
		if (pCurrentFree->sizeInBytes >= totalAllocationSize + padding + sizeof(FreeEntry))
		{
			//Create new FreeEntry after the allocation
			FreeEntry* pNewFreeEntry = new((void*)(aligned + totalAllocationSize)) FreeEntry(pCurrentFree->sizeInBytes - (totalAllocationSize + padding), pCurrentFree->pNext);

			//If last free is nullptr we are at start of FreeList
			if (pLastFree != nullptr)
				pLastFree->pNext = pNewFreeEntry;
			else
				m_pFreeList = pNewFreeEntry;

			//Create a new Allocation at the CurrentFree Address
			Allocation* pAllocation = new(pCurrentFree) Allocation(totalAllocationSize + padding);

			//Return the address of the allocation, but offset it so the size member does not get overridden.
			RegisterAllocation(totalAllocationSize, padding, (size_t)pAllocation, tag);
			return (void*)(aligned);
		}
		//Perfect Fit Free Block (No need to create a new FreeEntry after the allocation)
		else if (pCurrentFree->sizeInBytes == totalAllocationSize + padding)
		{
			//If last free is nullptr we are at start of FreeList
			if (pLastFree != nullptr)
				pLastFree->pNext = pCurrentFree->pNext;
			else
				m_pFreeList = pCurrentFree->pNext;

			//Create a new Allocation at the CurrentFree Address
			Allocation* pAllocation = new(pCurrentFree) Allocation(totalAllocationSize + padding);

			//Return the address of the allocation, but offset it so the size member does not get overridden.
			RegisterAllocation(totalAllocationSize, padding, (size_t)pAllocation, tag);
			return (void*)(aligned);
		}

		pLastFree = pCurrentFree;
		pCurrentFree = pCurrentFree->pNext;
	}

	return nullptr;
}

void MemoryManager::Free(void* allocation)
{
	Allocation* pAllocation = reinterpret_cast<Allocation*>(((size_t)allocation - sizeof(size_t))); //HACKING

	std::lock_guard<SpinLock> lock(m_MemoryLock);

	RemoveAllocation((size_t)pAllocation);
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
