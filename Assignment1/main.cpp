#include <iostream>
#include <list>
#include <mutex>

template <typename T>
class PoolAllocator
{
public:
	using ID = unsigned int;

	PoolAllocator() = default;
	~PoolAllocator() = default;

	T* Allocate();
	void Free(T*);

private:
	std::mutex m_Mutex;
	std::list<ID> m_FreeList;
	T* m_pMemory;
};


int main(int argc, const char* argv[])
{   
	std::cout << "Hello World" << std::endl;
	std::cin.get();
    return 0; 
}