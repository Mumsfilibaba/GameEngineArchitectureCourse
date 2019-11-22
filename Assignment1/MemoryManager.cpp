#include "MemoryManager.h"

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
	
	s_TotalAllocated = SIZE_IN_BYTES;
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

void MemoryManager::RegisterAllocation(
	const std::string& tag, 
	size_t startAddress, 
	size_t returnedMemoryAddress, 
	size_t endAddress, 
	size_t blockSizeInBytes, 
	size_t allocationSizeInBytes, 
	size_t alignment, 
	size_t internalPadding,
	size_t externalPadding,
	size_t extraMemory)
{
	constexpr size_t mb = 1024 * 1024;
	constexpr size_t kb = 1024;

	/*size_t blockSize_mb = blockSizeInBytes / mb;
	size_t blockSize_kb = (blockSizeInBytes - blockSize_mb * mb) / kb;
	size_t blockSize_bytes = (blockSizeInBytes - blockSize_mb * mb - blockSize_kb * kb);

	size_t allocationSize_mb = allocationSizeInBytes / mb;
	size_t allocationSize_kb = (allocationSizeInBytes - allocationSize_mb * mb) / kb;
	size_t allocationSize_bytes = (allocationSizeInBytes - allocationSize_mb * mb - allocationSize_kb * kb);

	size_t extraMemory_mb = extraMemory / mb;
	size_t extraMemory_kb = (extraMemory - extraMemory_mb * mb) / kb;
	size_t extraMemory_bytes = (extraMemory - extraMemory_mb * mb - extraMemory_kb * kb);*/

	/*std::stringstream stream;
	stream << "Start Address: " << std::setw(25) << "0x" << std::hex << startAddress  << std::endl;
	stream << "Allocation Address (Start + Padding): " << std::setw(0) << "0x" << std::hex << allocationAddress << std::endl;
	stream << "Returned Memory Address: " << std::setw(15) << "0x" << std::hex << returnedMemoryAddress << std::endl;
	stream << "End Address: " << std::setw(27) << "0x" << std::hex << endAddress << std::endl;*/

	std::stringstream threadId;
	threadId << std::this_thread::get_id();

	std::string info(512, ' ');
	info =
		"A" + tag + " [Thread ID: " + threadId.str() + "]" +
		"\nStart Address:                        " + N2HexStr(startAddress) +
		"\nReturned Memory Address:              " + N2HexStr(returnedMemoryAddress) +
		"\nEnd Address:                          " + N2HexStr(endAddress) +
		"\nTotal Block Size: " + std::to_string(blockSizeInBytes) +
		/*std::to_string(blockSize_mb) + "MB " +
		std::to_string(blockSize_kb) + "kB " +
		std::to_string(blockSize_bytes) + "bytes" +*/
		"\nAllocation Size: " + std::to_string(allocationSizeInBytes) +
		/*std::to_string(allocationSize_mb) + "MB " +
		std::to_string(allocationSize_kb) + "kB " +
		std::to_string(allocationSize_bytes) + "bytes" +*/
		"\nAlignment: " + std::to_string(alignment) +
		"\nInternal Padding: " + std::to_string(internalPadding) + " bytes" +
		"\nExternal Padding: " + std::to_string(externalPadding) + " bytes" +
		"\nExtra Memory: " + std::to_string(extraMemory)
		/*std::to_string(extraMemory_mb) + "MB " +
		std::to_string(extraMemory_kb) + "kB " +
		std::to_string(extraMemory_bytes) + "bytes"*/;
	m_AllocationsInfo[startAddress] = info;
}

void MemoryManager::RemoveAllocation(size_t startAddress)
{
	m_AllocationsInfo.erase(startAddress);
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
		size_t padding = (-offset & (alignment - 1));
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
			m_AllocationHeaders[aligned] = Allocation(allocationSizeInBytes + internalPadding, internalPadding);

			//Return the address of the allocation, but offset it so the size member does not get overridden.
			RegisterAllocation(
				tag, 
				(size_t)pCurrentFree + externalPadding,
				aligned, 
				aligned + allocationSizeInBytes - 1, 
				padding + allocationSizeInBytes,
				allocationSizeInBytes,
				alignment,
				internalPadding,
				externalPadding,
				0);
			
			s_TotalUsed += (allocationSizeInBytes + padding);
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
	RemoveAllocation(allocationAddress);

	size_t offsetAllocationAddress = allocationAddress - allocation.padding;

	FreeEntry* pLastFree = nullptr;
	FreeEntry* pCurrentFree = m_pFreeListStart;

	do
	{
		//We encounter a block that ends in our start (Coalesce left)
		if ((size_t)pCurrentFree + pCurrentFree->sizeInBytes == offsetAllocationAddress)
		{
			//We have a block on our right that we can coalesce with as well
			if (offsetAllocationAddress + allocation.sizeInBytes == (size_t)pCurrentFree->pNext)
			{
				size_t newFreeSize = pCurrentFree->sizeInBytes + allocation.sizeInBytes + pCurrentFree->pNext->sizeInBytes;

				FreeEntry* pCurrentFreeNextNext = pCurrentFree->pNext->pNext;
				FreeEntry* pNewFreeEntry = new(pCurrentFree) FreeEntry(newFreeSize);
				pLastFree->pNext = pNewFreeEntry;
				pNewFreeEntry->pNext = pCurrentFreeNextNext;
			}
			else
			{
				size_t newFreeSize = pCurrentFree->sizeInBytes + allocation.sizeInBytes;

				FreeEntry* pCurrentFreeNext = pCurrentFree->pNext;
				FreeEntry* pNewFreeEntry = new(pCurrentFree) FreeEntry(newFreeSize);
				pLastFree->pNext = pNewFreeEntry;
				pNewFreeEntry->pNext = pCurrentFreeNext;
			}

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

			return;
		}

		pLastFree = pCurrentFree;
		pCurrentFree = pCurrentFree->pNext;

	} while (pCurrentFree != m_pFreeListStart);
}
