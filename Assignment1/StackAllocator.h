#ifndef STACKALLOCATOR_H
#define STACKALLOCATOR_H

class StackAllocator
{
private:
	void* m_pStart;
	void* m_pEnd;
	void* m_pCurrent;
	size_t toRemove;
public:
	StackAllocator(size_t size);
	~StackAllocator();
	void free(void* ptr);
	void* getStart();

	template <class T>
	void make_delete(const T *ptr);
	template <class T>
	void *make_new(const T &object);

	void* allocate(size_t size);
};

#endif

inline StackAllocator::StackAllocator(size_t size)
{
	m_pStart = malloc(size);
	m_pEnd = (char*)m_pStart + size;
	m_pCurrent = nullptr;
	toRemove = 0;
}

inline StackAllocator::~StackAllocator()
{
	delete m_pStart;
}

inline void * StackAllocator::allocate(size_t size)
{
	if (toRemove > 0)
		free(m_pCurrent);

	if (m_pCurrent != nullptr)
		m_pCurrent = (char*)m_pCurrent + size;
	else
		m_pCurrent = m_pStart;

	if (size + (char*)m_pCurrent > (char*)m_pEnd)
	{
		//To much memory has been used!
		return nullptr;
	}

	return m_pCurrent;
}

inline void StackAllocator::free(void * ptr)
{
	// Free everything up until that pointer!

	if (ptr < m_pEnd && ptr >= m_pStart && ptr <= m_pCurrent)
	{
		m_pCurrent = (char*)ptr - toRemove;
		toRemove = 0;
	}
}

inline void * StackAllocator::getStart()
{
	return m_pStart;
}

template<class T>
inline void StackAllocator::make_delete(const T * ptr)
{
	if (ptr)
	{
		toRemove += sizeof(T);
		((T*)ptr)->~T();
	}
}

template<class T>
inline void * StackAllocator::make_new(const T &object)
{
	void* res = allocate(sizeof(T));
	new (res) T(object);
	return res;
}
