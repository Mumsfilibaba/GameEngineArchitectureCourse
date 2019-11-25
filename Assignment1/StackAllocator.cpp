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

	s_TotalAllocated += m_Size;
}

StackAllocator::~StackAllocator()
{
	s_TotalAllocated -= m_Size;
}

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
	s_TotalUsed += size + padding;

#ifdef SHOW_ALLOCATIONS_DEBUG
	MemoryManager::GetInstance().RegisterStackAllocation(tag, (size_t)pMemory, size);
#endif
    return pMemory;
}

void StackAllocator::Reset()
{
	m_pCurrent = m_pStart;

	s_TotalUsed -= m_Used;
	m_Used = 0;

#ifdef SHOW_ALLOCATIONS_DEBUG
	MemoryManager::GetInstance().ClearStackAllocations();
#endif
}
