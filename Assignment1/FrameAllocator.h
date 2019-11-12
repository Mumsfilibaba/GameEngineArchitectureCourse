#ifndef FRAMEALLOCATOR_H
#define FRAMEALLOCATOR_H

class FrameAllocator
{
private:
	char* m_pStart;
	char* m_pEnd;
	char* m_pCurrent;
public:
	FrameAllocator(size_t size);
	FrameAllocator(char* start, char* end);
	~FrameAllocator();

	template<class T>
	char* allocate(const T &object);
	template<class T>
	T* allocateArray(unsigned int size);
	void reset();
};

#endif

template<class T>
inline char * FrameAllocator::allocate(const T &object)
{
	char* res = nullptr;
	size_t size = sizeof(object);
	if (m_pCurrent + size < m_pEnd)
	{
		res = m_pCurrent;
		m_pCurrent += size;
		new (res) T(object);
	}
	return res;
}

template<class T>
inline T * FrameAllocator::allocateArray(unsigned int size)
{
	T* res = nullptr;
	size_t arrSize = sizeof(T) * size;

	if (m_pCurrent + arrSize < m_pEnd)
	{
		res = (T*)m_pCurrent;
		m_pCurrent += arrSize;
		new (res) T[size];
	}
	return res;
}
