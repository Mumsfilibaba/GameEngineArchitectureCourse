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
	~FrameAllocator();

	template<class T>
	char * allocate(const T &object);
	void free();
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
