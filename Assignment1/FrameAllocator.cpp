#include "FrameAllocator.h"
#include "MemoryManager.h"

std::unordered_map<std::thread::id, FrameAllocator*> FrameAllocator::s_FrameAllocatorMap = std::unordered_map<std::thread::id, FrameAllocator*>();

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

void FrameAllocator::reset()
{
	m_pCurrent = m_pStart;
}
