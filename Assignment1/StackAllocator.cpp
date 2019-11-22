#include "StackAllocator.h"
#include "MemoryManager.h"

int StackAllocator::s_TotalAllocated = 0;
int StackAllocator::s_TotalUsed = 0;

StackAllocator::StackAllocator(size_t size)
	: m_Size(size),
	m_Used(0)
{
	m_pStart = allocate(size, 1, "Stack Allocator");
	m_pEnd = (void*)((size_t)m_pStart + size);
	m_pCurrent = m_pStart;

	s_TotalAllocated += m_Size;
}

StackAllocator::~StackAllocator()
{
	s_TotalAllocated -= m_Size;
}

void* StackAllocator::AllocateMemory(size_t size, size_t alignment)
{
    size_t mask = alignment - 1;
    size_t alignedCurrent = ((size_t)m_pCurrent + mask) & ~mask;
    size_t padding = alignedCurrent - (size_t)m_pCurrent;
    
    void* pMemory = nullptr;
    if ((void*)((size_t)m_pCurrent + size) <= m_pEnd)
    {
        pMemory = m_pCurrent;
        m_pCurrent = (void*)((size_t)m_pCurrent + size);
    }

	m_Used		+= size + padding;
	s_TotalUsed += size + padding;
    return pMemory;
}

void StackAllocator::Reset()
{
	m_pCurrent = m_pStart;

	s_TotalUsed -= m_Used;
	m_Used = 0;
}
