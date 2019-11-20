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

void MemoryManager::RegisterAllocation(
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
	size_t extraMemory)
{
	size_t blockSize_mb = blockSizeInBytes / (1024 * 1024);
	size_t blockSize_kb = (blockSizeInBytes - blockSize_mb * (1024 * 1024)) / 1024;
	size_t blockSize_bytes = (blockSizeInBytes - blockSize_mb * (1024 * 1024) - blockSize_kb * 1024);

	size_t allocationSize_mb = allocationSizeInBytes / (1024 * 1024);
	size_t allocationSize_kb = (allocationSizeInBytes - allocationSize_mb * (1024 * 1024)) / 1024;
	size_t allocationSize_bytes = (allocationSizeInBytes - allocationSize_mb * (1024 * 1024) - allocationSize_kb * 1024);

	size_t extraMemory_mb = extraMemory / (1024 * 1024);
	size_t extraMemory_kb = (extraMemory - extraMemory_mb * (1024 * 1024)) / 1024;
	size_t extraMemory_bytes = (extraMemory - extraMemory_mb * (1024 * 1024) - extraMemory_kb * 1024);

	std::stringstream stream;
	stream << "Start Address: " << std::setw(25) << "0x" << std::hex << startAddress  << std::endl;
	stream << "Allocation Address (Start + Padding): " << std::setw(0) << "0x" << std::hex << allocationAddress << std::endl;
	stream << "Returned Memory Address: " << std::setw(15) << "0x" << std::hex << returnedMemoryAddress << std::endl;
	stream << "End Address: " << std::setw(27) << "0x" << std::hex << endAddress << std::endl;

	m_Allocations[startAddress] =
		"A" + tag + "\n" +
		stream.str() +
		"Total Block Size: " +
		std::to_string(blockSize_mb) + "MB " +
		std::to_string(blockSize_kb) + "kB " +
		std::to_string(blockSize_bytes) + "bytes" +
		"\nAllocation Size: " +
		std::to_string(allocationSize_mb) + "MB " +
		std::to_string(allocationSize_kb) + "kB " +
		std::to_string(allocationSize_bytes) + "bytes" +
		"\nAlignment: " + std::to_string(alignment) +
		"\nInternal Padding: " + std::to_string(internalPadding) + " bytes"
		"\nExternal Padding: " + std::to_string(externalPadding) + " bytes"
		"\nExtra Memory: " + 
		std::to_string(extraMemory_mb) + "MB " +
		std::to_string(extraMemory_kb) + "kB " +
		std::to_string(extraMemory_bytes) + "bytes";
}

void MemoryManager::RemoveAllocation(size_t startAddress)
{
	m_Allocations.erase(startAddress);
}

void* MemoryManager::Allocate(size_t sizeInBytes, size_t alignment, const std::string& tag)
{
	assert(alignment % 2 == 0 || alignment == 1);

	size_t totalAllocationSize = sizeInBytes + sizeof(Allocation);

	std::lock_guard<SpinLock> lock(m_MemoryLock);

	FreeEntry* pLastFree = nullptr;
	FreeEntry* pCurrentFree = m_pFreeList;

	while (pCurrentFree != nullptr)
	{
		size_t offset = (size_t)pCurrentFree + sizeof(Allocation);
		size_t padding = (-offset & (alignment - 1));
		size_t aligned = offset + padding;
		//Not Perfect Fit Free Block (Need to create a new FreeEntry after the allocation)
		if (pCurrentFree->sizeInBytes >= totalAllocationSize + padding + sizeof(FreeEntry))
		{
			//Create new FreeEntry after the allocation
			FreeEntry* pNewFreeEntryAfter = new((void*)(aligned + sizeInBytes)) FreeEntry(pCurrentFree->sizeInBytes - (totalAllocationSize + padding), pCurrentFree->pNext);

			//If last free is nullptr we are at start of FreeList
			if (pLastFree != nullptr)
				pLastFree->pNext = pNewFreeEntryAfter;
			else
				m_pFreeList = pNewFreeEntryAfter;

			size_t internalPadding = padding;
			size_t externalPadding = 0;

			//Check if Padding can fit a Free Block
			if (padding > sizeof(FreeEntry))
			{
				FreeEntry* pNewFreeEntryBefore = new(pCurrentFree) FreeEntry(padding, pNewFreeEntryAfter);

				if (pLastFree != nullptr)
					pLastFree->pNext = pNewFreeEntryBefore;
				else
					m_pFreeList = pNewFreeEntryBefore;

				externalPadding = internalPadding;
				internalPadding = 0;
			}

			//Create a new Allocation at the CurrentFree Address
			Allocation* pAllocation = new((void*)((size_t)pCurrentFree + padding)) Allocation(totalAllocationSize + internalPadding, internalPadding);

			//Return the address of the allocation, but offset it so the size member does not get overridden.
			RegisterAllocation(
				tag, 
				(size_t)pCurrentFree + externalPadding, 
				(size_t)pCurrentFree + padding,
				aligned, 
				aligned + sizeInBytes - 1, 
				padding + totalAllocationSize,
				sizeInBytes,
				alignment,
				internalPadding,
				externalPadding,
				0);
			return (void*)(aligned);
		}
		//Cant fit new Free Block in, but we can fit the allocation in. (We give the allocation some extra memory at the end)
		else if (pCurrentFree->sizeInBytes >= totalAllocationSize + padding)
		{
			//If last free is nullptr we are at start of FreeList
			if (pLastFree != nullptr)
				pLastFree->pNext = pCurrentFree->pNext;
			else
				m_pFreeList = pCurrentFree->pNext;

			size_t internalPadding = padding;
			size_t externalPadding = 0;

			//Check if Padding can fit a Free Block
			if (padding > sizeof(FreeEntry))
			{
				FreeEntry* pNewFreeEntryBefore = new(pCurrentFree) FreeEntry(padding, pCurrentFree->pNext);

				if (pLastFree != nullptr)
					pLastFree->pNext = pNewFreeEntryBefore;
				else
					m_pFreeList = pNewFreeEntryBefore;

				externalPadding = internalPadding;
				internalPadding = 0;
			}

			//Create a new Allocation at the CurrentFree Address
			Allocation* pAllocation = new((void*)((size_t)pCurrentFree + padding)) Allocation(pCurrentFree->sizeInBytes - externalPadding, internalPadding);

			//Return the address of the allocation, but offset it so the size member does not get overridden.
			RegisterAllocation(
				tag,
				(size_t)pCurrentFree + externalPadding,
				(size_t)pCurrentFree + padding,
				aligned,
				(size_t)pCurrentFree + pCurrentFree->sizeInBytes - 1,
				pCurrentFree->sizeInBytes,
				sizeInBytes,
				alignment,
				internalPadding,
				externalPadding,
				pCurrentFree->sizeInBytes - totalAllocationSize + padding);
			return (void*)(aligned);
		}

		pLastFree = pCurrentFree;
		pCurrentFree = pCurrentFree->pNext;
	}

	return nullptr;
}

void MemoryManager::Free(void* allocation)
{
	size_t allocationAddress = (size_t)allocation - sizeof(Allocation);
	Allocation* pAllocation = reinterpret_cast<Allocation*>(allocationAddress); //HACKING

	std::lock_guard<SpinLock> lock(m_MemoryLock);

	RemoveAllocation((size_t)pAllocation);

	FreeEntry* pClosestLeft = nullptr;

	FreeEntry* pLastFree = nullptr;
	FreeEntry* pCurrentFree = m_pFreeList;

	while (pCurrentFree != nullptr)
	{
		if (allocationAddress + pAllocation->sizeInBytes == (size_t)pCurrentFree)
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
