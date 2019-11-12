#include "FrameAllocator.h"
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
