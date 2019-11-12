#ifndef STACKALLOCATOR_H
#define STACKALLOCATOR_H
template<class T>
class StackAllocator
{
private:
	void* m_pStart;
	void* m_pEnd;
	void* m_pCurrent;
public:
	StackAllocator(void* start, void* end);
	T* allocate();
	void free(void* ptr);
};

#endif

template<class T>
inline StackAllocator<T>::StackAllocator(void * start, void * end): m_pStart(start), m_pEnd(end), m_pCurrent(nullptr)
{
}

template<class T>
inline T * StackAllocator<T>::allocate()
{
	size_t size = sizeof(T);
	if (m_pCurrent != nullptr)
		m_pCurrent = (char*)m_pCurrent + size;
	else
		m_pCurrent = m_pStart;

	if (size + (char*)m_pCurrent > (char*)m_pEnd)
	{
		//To much memory has been used!
		return nullptr;
	}

	return (T*)m_pCurrent;
}

template<class T>
inline void StackAllocator<T>::free(void * ptr)
{
	// Free everything up until that pointer!
	if (ptr < m_pEnd && ptr >= m_pStart && ptr <= m_pCurrent)
	{
		m_pCurrent = (char*)ptr - sizeof(T);
	}
}
