#include "MemoryManager.h"
#include <thread>
#include <iostream>
#ifdef _WIN32
//include minimal windows headers
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#endif

std::atomic_size_t MemoryManager::s_TotalAllocated = 0;
std::atomic_size_t MemoryManager::s_TotalUsed = 0;

#define DEBUG_MEMORY_MANAGER

#if defined(DEBUG_MEMORY_MANAGER) && defined(_WIN32)
inline std::ostream& redText(std::ostream& s)
{
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hStdout,
		FOREGROUND_RED | FOREGROUND_INTENSITY);
	return s;
}

inline std::ostream& greenText(std::ostream& s)
{
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hStdout,
		FOREGROUND_GREEN | FOREGROUND_INTENSITY);
	return s;
}

inline std::ostream& blueText(std::ostream& s)
{
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hStdout, FOREGROUND_BLUE
		| FOREGROUND_GREEN | FOREGROUND_INTENSITY);
	return s;
}

inline std::ostream& yellowText(std::ostream& s)
{
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hStdout,
		FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY);
	return s;
}

inline std::ostream& whiteText(std::ostream& s)
{
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hStdout,
		FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
	return s;
}
#endif

MemoryManager::MemoryManager() :
	m_pMemory(malloc(SIZE_IN_BYTES))
{
	m_pFreeTail = new(m_pMemory) FreeEntry(SIZE_IN_BYTES / 2);
	m_pFreeHead = new((void*)((size_t)m_pMemory + SIZE_IN_BYTES / 2)) FreeEntry(SIZE_IN_BYTES / 2);
	m_pFreeTail->pNext = m_pFreeHead;
	m_pFreeHead->pNext = m_pFreeTail;

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

	m_pFreeHead = nullptr;
	m_pFreeTail = nullptr;

	m_AllocationHeaders.clear();
}

void MemoryManager::PrintMemoryLayout()
{
#ifdef DEBUG_MEMORY_MANAGER
	FreeEntry* pLastFree = m_pFreeTail;
	FreeEntry* pCurrentFree = m_pFreeHead;

	do
	{
		if ((size_t)pCurrentFree < (size_t)pLastFree)
			break;

		pLastFree = pCurrentFree;
		pCurrentFree = pCurrentFree->pNext;

	} while (pCurrentFree != m_pFreeHead);

	FreeEntry* pTrueTail = pLastFree;
	FreeEntry* pTrueStart = pCurrentFree;
	size_t prevNext = 0;

	auto& allocationIt = m_AllocationHeaders.begin();

	do
	{
		if (prevNext != (size_t)pCurrentFree && prevNext > 0)
		{
			while (allocationIt != m_AllocationHeaders.end())
			{
				if (allocationIt->first >= (size_t)pCurrentFree)
					break;

				std::cout << yellowText << N2HexStr(allocationIt->first) << " : " << allocationIt->second.tag << std::endl;
				allocationIt++;
			}
		}
			

		size_t next = (size_t)pCurrentFree + pCurrentFree->sizeInBytes;
		std::cout << blueText << N2HexStr((size_t)pCurrentFree) << " next: " << N2HexStr(next) << " points to: " << N2HexStr((size_t)pCurrentFree->pNext);

		if (pCurrentFree == m_pFreeHead)
			std::cout << " <-- H";
		else if (pCurrentFree == m_pFreeTail)
			std::cout << " <-- T";

		std::cout << std::endl;

		prevNext = next;

		pLastFree = pCurrentFree;
		pCurrentFree = pCurrentFree->pNext;

	} while (pCurrentFree != pTrueStart);

	std::cout << whiteText << std::endl;
#endif
}

void MemoryManager::CheckFreeListCorruption()
{
#ifdef DEBUG_MEMORY_MANAGER
	std::cout << "---------------------START---------------------" << std::endl;

	PrintMemoryLayout();

	FreeEntry* pLastFree = m_pFreeTail;
	FreeEntry* pCurrentFree = m_pFreeHead;

	do
	{
		if ((size_t)pCurrentFree < (size_t)pLastFree)
			break;

		pLastFree = pCurrentFree;
		pCurrentFree = pCurrentFree->pNext;

	} while (pCurrentFree != m_pFreeHead);

	FreeEntry* pTrueTail = pLastFree;
	FreeEntry* pTrueStart = pCurrentFree;

	std::cout << "True Start: " << N2HexStr((size_t)pTrueStart) << std::endl;

	do
	{
		size_t offset = (size_t)pLastFree + pLastFree->sizeInBytes;
		size_t next = (size_t)pCurrentFree;

		if (offset >= next && pCurrentFree != pTrueStart)
		{
			FreeEntry* pCorruptionLast = pLastFree;
			FreeEntry* pCorruptionCurrent = pCurrentFree;

			pLastFree = pTrueTail;
			pCurrentFree = pTrueStart;

			do
			{
				if (pCurrentFree == pCorruptionLast)
				{
					std::cout << redText << N2HexStr((size_t)pCurrentFree) << " next: " << N2HexStr((size_t)pCurrentFree + pCurrentFree->sizeInBytes) << " points to: " << N2HexStr((size_t)pCurrentFree->pNext) << " <-- L" << std::endl;
				}
				else if (pCurrentFree == pCorruptionCurrent)
				{
					std::cout << redText << N2HexStr((size_t)pCurrentFree) << " next: " << N2HexStr((size_t)pCurrentFree + pCurrentFree->sizeInBytes) << " points to: " << N2HexStr((size_t)pCurrentFree->pNext) << " <-- C" << std::endl;
				}
				else
				{
					std::cout << greenText << N2HexStr((size_t)pCurrentFree) << " next: " << N2HexStr((size_t)pCurrentFree + pCurrentFree->sizeInBytes) << " points to: " << N2HexStr((size_t)pCurrentFree->pNext) << std::endl;
				}

				pLastFree = pCurrentFree;
				pCurrentFree = pCurrentFree->pNext;

			} while (pCurrentFree != pTrueStart);

			std::cout << whiteText << "------------CORRUPTION DETECTED END------------" << std::endl << std::endl;
			return;
		}

		pLastFree = pCurrentFree;
		pCurrentFree = pCurrentFree->pNext;

	} while (pCurrentFree != pTrueStart);

	std::cout << whiteText << "----------------------END----------------------" << std::endl << std::endl;
#endif
}

void* MemoryManager::Allocate(size_t allocationSizeInBytes, size_t alignment, const std::string& tag)
{
	assert(alignment % 2 == 0 || alignment == 1);
	assert(allocationSizeInBytes > 0);

	std::scoped_lock<SpinLock> lock(m_MemoryLock);

	PrintMemoryLayout();

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
#ifdef DEBUG_MEMORY_MANAGER
			std::cout << "Allocating Memory: " << N2HexStr(aligned) << " to: " << tag << std::endl;
			PrintMemoryLayout();
#endif

			FreeEntry* pCurrentFreeNext = pCurrentFree->pNext;

			size_t internalPadding = padding;

			FreeEntry* pNewFreeEntryBefore = nullptr;
			FreeEntry* pNewFreeEntryAfter = nullptr;

			if (pCurrentFree->sizeInBytes >= allocationSizeInBytes + padding + sizeof(FreeEntry))
			{
				//Create new FreeEntry after the allocation
				pNewFreeEntryAfter = new((void*)(aligned + allocationSizeInBytes)) FreeEntry(pCurrentFree->sizeInBytes - (allocationSizeInBytes + padding));
			}

			//Check if previous Free Block is our neighbor, if so coalesce padding with it.
			if ((size_t)pLastFree + pLastFree->sizeInBytes == (size_t)pCurrentFree)
			{
				pLastFree->sizeInBytes += padding;
				internalPadding = 0;
			}
			else if (padding > sizeof(FreeEntry)) //Check if Padding can fit a Free Block
			{
				pNewFreeEntryBefore = new(pCurrentFree) FreeEntry(padding);
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

			//Create a new allocation at the CurrentFree Address
			assert(m_AllocationHeaders.find(aligned) == m_AllocationHeaders.end());
			m_AllocationHeaders[aligned] = Allocation(tag, allocationSizeInBytes + internalPadding, internalPadding);

#ifdef DEBUG_MEMORY_MANAGER
			CheckFreeListCorruption();
#endif

#ifndef COLLECT_PERFORMANCE_DATA
			s_TotalUsed += (allocationSizeInBytes + internalPadding);
#endif
			//Return the address of the allocation
			return (void*)(aligned);
		}

		pLastFree = pCurrentFree;
		pCurrentFree = pCurrentFree->pNext;

	} while (pCurrentFree != m_pFreeHead);

	assert(false);
	return nullptr;
}

void MemoryManager::Free(void* allocationPtr)
{
	std::scoped_lock<SpinLock> lock(m_MemoryLock);

	assert(allocationPtr != nullptr);

	size_t allocationAddress = (size_t)allocationPtr;

	assert(m_AllocationHeaders.find(allocationAddress) != m_AllocationHeaders.end());

	Allocation allocation = m_AllocationHeaders[allocationAddress];
	m_AllocationHeaders.erase(allocationAddress);

#ifdef DEBUG_MEMORY_MANAGER
	std::cout << "Freeing Memory: " << N2HexStr(allocationAddress) << " from: " << allocation.tag << std::endl;
	PrintMemoryLayout();
#endif

#ifndef COLLECT_PERFORMANCE_DATA
	s_TotalUsed -= allocation.sizeInBytes;
#endif

	size_t offsetAllocationAddress = allocationAddress - allocation.padding;

	FreeEntry* pLastFree = m_pFreeTail;
	FreeEntry* pCurrentFree = m_pFreeHead;

	FreeEntry* pClosestLeft = nullptr;
	FreeEntry* pClosestRight = (FreeEntry*)ULLONG_MAX;

	//Check if we have one or more allocation left, if this is the last allocation we don't want to coalesce the last two freeblocks
	if (m_AllocationHeaders.size() >= 1)
	{
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

#ifdef DEBUG_MEMORY_MANAGER
					std::cout << "Coalesce left then right" << std::endl;
#endif
				}
				else
				{
					size_t newFreeSize = pCurrentFree->sizeInBytes + allocation.sizeInBytes;

					FreeEntry* pCurrentFreeNext = pCurrentFree->pNext;
					pNewFreeEntry = new(pCurrentFree) FreeEntry(newFreeSize);
					pLastFree->pNext = pNewFreeEntry;
					pNewFreeEntry->pNext = pCurrentFreeNext;

#ifdef DEBUG_MEMORY_MANAGER
					std::cout << "Coalesce left" << std::endl;
#endif

				}

				m_pFreeHead = pNewFreeEntry;
				m_pFreeTail = pLastFree;

#ifdef DEBUG_MEMORY_MANAGER
				CheckFreeListCorruption();
#endif
				return;
			}
			//We encounter a block that starts in our end (Coalesce right)
			else if (offsetAllocationAddress + allocation.sizeInBytes == (size_t)pCurrentFree)
			{
				FreeEntry* pNewFreeEntry = nullptr;

				//We have a block on our left that we can coalesce with as well
				if ((size_t)pLastFree + pLastFree->sizeInBytes == offsetAllocationAddress)
				{
					size_t newFreeSize = pLastFree->sizeInBytes + allocation.sizeInBytes + pCurrentFree->sizeInBytes;

					FreeEntry* pCurrentFreeNext = pCurrentFree->pNext;
					pNewFreeEntry = new(pLastFree) FreeEntry(newFreeSize);
					pNewFreeEntry->pNext = pCurrentFreeNext;


					m_pFreeHead = pNewFreeEntry->pNext;
					m_pFreeTail = pNewFreeEntry;

#ifdef DEBUG_MEMORY_MANAGER
					std::cout << "Coalesce Right then left" << std::endl;

					CheckFreeListCorruption();
#endif
				}
				else
				{
					size_t newFreeSize = allocation.sizeInBytes + pCurrentFree->sizeInBytes;

					FreeEntry* pCurrentFreeNext = pCurrentFree->pNext;
					pNewFreeEntry = new((void*)offsetAllocationAddress) FreeEntry(newFreeSize);
					pLastFree->pNext = pNewFreeEntry;
					pNewFreeEntry->pNext = pCurrentFreeNext;


					m_pFreeHead = pNewFreeEntry;
					m_pFreeTail = pLastFree;

#ifdef DEBUG_MEMORY_MANAGER
					std::cout << "Coalesce Right" << std::endl;

					CheckFreeListCorruption();
#endif
				}
				return;
			}

			if ((size_t)pLastFree < allocationAddress)
			{
				if ((size_t)pCurrentFree > allocationAddress)
				{
					pClosestLeft = pLastFree;
					pClosestRight = pCurrentFree;
					//break;
				}
				else if ((size_t)pCurrentFree < (size_t)pLastFree && (size_t)pCurrentFree < (size_t)pClosestRight)
				{
					pClosestLeft = pLastFree;
					pClosestRight = pCurrentFree;
					//break;
				}
			}

			pLastFree = pCurrentFree;
			pCurrentFree = pCurrentFree->pNext;

		} while (pCurrentFree != m_pFreeHead);
	}

	if (pClosestLeft == nullptr)
	{
		pClosestLeft = m_pFreeTail;
		pClosestRight = m_pFreeHead;
	}

	FreeEntry* pNewFreeEntry = new((void*)offsetAllocationAddress) FreeEntry(allocation.sizeInBytes);
	pClosestLeft->pNext = pNewFreeEntry;
	pNewFreeEntry->pNext = pClosestRight;


	m_pFreeHead = pNewFreeEntry;
	m_pFreeTail = pClosestLeft;

#ifdef DEBUG_MEMORY_MANAGER
	std::cout << "Single Free" << std::endl;
	CheckFreeListCorruption();
#endif
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
