#include "StackAllocator.h"
#include "MemoryManager.h"

StackAllocator::StackAllocator(size_t size)
{
	m_pStart = MemoryManager::GetInstance().Allocate(size, 1, "Stack Allocator");
	m_pEnd = (void*)((size_t)m_pStart + size);
	m_pCurrent = m_pStart;
}

StackAllocator::~StackAllocator()
{
	
}

void* StackAllocator::AllocateMem(size_t size, size_t alignment)
{
    size_t mask = alignment - 1;
    size_t alignedCurrent = ((size_t)m_pCurrent + mask) & ~mask;
    size_t padding = alignedCurrent - (size_t)m_pCurrent;
    
    void* res = nullptr;
    if ((void*)((size_t)m_pCurrent + size) <= m_pEnd)
    {
        res = m_pCurrent;
        m_pCurrent = (void*)((size_t)m_pCurrent + size);
    }
    return res;
}

void StackAllocator::Reset()
{
	m_pCurrent = m_pStart;
}
