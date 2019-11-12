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
	char * allocate(const T &object, size_t sizeOfObject);
	void free();
};

#endif

template<class T>
inline char * FrameAllocator::allocate(const T &object, size_t sizeOfObject)
{
	char* res = nullptr;
	if (m_pCurrent + sizeOfObject < m_pEnd)
	{
		res = m_pCurrent;
		m_pCurrent += sizeOfObject;
		new (res) T(object);
	}
	return res;
}
