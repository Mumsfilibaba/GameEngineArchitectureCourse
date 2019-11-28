#include "StackAllocator.h"
#include "MemoryManager.h"

std::atomic_int32_t StackAllocator::s_TotalAllocated = 0;
std::atomic_int32_t StackAllocator::s_TotalUsed = 0;

StackAllocator::StackAllocator(size_t size)
	: m_Size(size),
	m_Used(0)
{
	m_pStart = allocate(size, 1, "Stack Allocation Chunk");
	m_pEnd = (void*)((size_t)m_pStart + size);
	m_pCurrent = m_pStart;

#ifndef COLLECT_PERFORMANCE_DATA
	s_TotalAllocated += m_Size;
#endif
}

StackAllocator::~StackAllocator()
{
#ifndef COLLECT_PERFORMANCE_DATA
	s_TotalAllocated -= m_Size;
#endif
}

#ifdef SHOW_ALLOCATIONS_DEBUG
void* StackAllocator::AllocateMemory(const std::string& tag, size_t size, size_t alignment)
{
    size_t mask = alignment - 1;
    size_t alignedCurrent = ((size_t)m_pCurrent + mask) & ~mask;
    size_t padding = alignedCurrent - (size_t)m_pCurrent;
    
    void* pMemory = nullptr;
    if ((void*)((size_t)alignedCurrent + size) <= m_pEnd)
    {
        pMemory = (void*)alignedCurrent;
        m_pCurrent = (void*)((size_t)alignedCurrent + size);
    }

	m_Used		+= size + padding;

#ifndef COLLECT_PERFORMANCE_DATA
	s_TotalUsed += size + padding;
#endif

	MemoryManager::GetInstance().RegisterStackAllocation(tag, (size_t)pMemory, size);
    return pMemory;
}
#else
void* StackAllocator::AllocateMemory(size_t size, size_t alignment)
{
	size_t mask = alignment - 1;
	size_t alignedCurrent = ((size_t)m_pCurrent + mask) & ~mask;
	size_t padding = alignedCurrent - (size_t)m_pCurrent;

	void* pMemory = nullptr;
	if ((void*)((size_t)alignedCurrent + size) <= m_pEnd)
	{
		pMemory = (void*)alignedCurrent;
		m_pCurrent = (void*)((size_t)alignedCurrent + size);
	}

	m_Used += size + padding;

#ifndef COLLECT_PERFORMANCE_DATA
	s_TotalUsed += size + padding;
#endif

	return pMemory;
}
#endif

void StackAllocator::Reset()
{
	m_pCurrent = m_pStart;
#ifndef COLLECT_PERFORMANCE_DATA
	s_TotalUsed -= m_Used;
#endif
	m_Used = 0;

#ifdef SHOW_ALLOCATIONS_DEBUG
	MemoryManager::GetInstance().ClearStackAllocations();
#endif
}
