#ifndef FRAMEALLOCATOR_H
#define FRAMEALLOCATOR_H
//Objects allocated through this allocator will never have their destruct called from it. Therefore it is up to the user to call upon the destructor before freeing the memory!

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

	template<class T, typename... Args>
	T* allocate(Args&& ... args);
	template<class T>
	T* allocateArray(unsigned int size);
	void reset();
};

#endif

template<class T, typename ... Args>
inline T * FrameAllocator::allocate(Args&&... args)
{
	T* res = nullptr;
	size_t size = sizeof(T);
	if (m_pCurrent + size <= m_pEnd)
	{
		res = (T*)m_pCurrent;
		m_pCurrent += size;
		new (res) T(std::forward<Args>(args) ...);
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
