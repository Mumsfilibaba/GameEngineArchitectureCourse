#include "FrameAllocator.h"

std::unordered_map<std::thread::id, FrameAllocator*> FrameAllocator::s_FrameAllocatorMap = std::unordered_map<std::thread::id, FrameAllocator*>();
SpinLock FrameAllocator::m_InstanceLock;

FrameAllocator::FrameAllocator(size_t size)
{
	m_pStart = new char[size];
	m_pEnd = m_pStart + size;
	m_pCurrent = m_pStart;
}

FrameAllocator::FrameAllocator(char * start, char * end): m_pStart(start), m_pEnd(end), m_pCurrent(start)
{
}

FrameAllocator::~FrameAllocator()
{
	delete[] m_pStart;
}

void FrameAllocator::reset()
{
	m_pCurrent = m_pStart;
}
