#include "FrameAllocator.h"

FrameAllocator::FrameAllocator(size_t size)
{
	m_pStart = new char[size];
	m_pEnd = m_pStart + size;
	m_pCurrent = m_pStart;
}

FrameAllocator::~FrameAllocator()
{
	delete[] m_pStart;
}

void FrameAllocator::free()
{
	m_pCurrent = m_pStart;
}
