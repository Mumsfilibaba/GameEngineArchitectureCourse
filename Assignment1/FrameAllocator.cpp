#include "FrameAllocator.h"
#include "MemoryManager.h"

std::unordered_map<std::thread::id, FrameAllocator*> FrameAllocator::s_FrameAllocatorMap = std::unordered_map<std::thread::id, FrameAllocator*>();
SpinLock FrameAllocator::m_InstanceLock;

FrameAllocator::FrameAllocator(size_t size)
{
	//m_pStart = malloc(size);
	m_pStart = MemoryManager::GetInstance().Allocate(size, "Frame Allocator");
	m_pEnd = (void*)((size_t)m_pStart + size);
	m_pCurrent = m_pStart;
}

FrameAllocator::FrameAllocator(void* start, void* end) 
:	m_pStart(start),
m_pEnd(end),
m_pCurrent(start)
{
}


FrameAllocator::~FrameAllocator()
{
	//delete[] m_pStart;
}


void* FrameAllocator::AllocateMem(size_t size, size_t alignment)
{
    size_t mask = alignment - 1;
    size_t alignedCurrent = ((size_t)m_pCurrent + mask) & ~mask;
    size_t padding = alignedCurrent - (size_t)m_pCurrent;
    
    void* res = nullptr;
    if (m_pCurrent + size <= m_pEnd)
    {
        res = m_pCurrent;
        m_pCurrent += size;
    }
    return res;
}


void FrameAllocator::Reset()
{
	m_pCurrent = m_pStart;
}
