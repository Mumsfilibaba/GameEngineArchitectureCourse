#pragma once
#include <unordered_map>
#include <thread>
#include "MemoryManager.h"

#include "SpinLock.h"

//Objects allocated through this allocator will never have their destruct called from it. Therefore it is up to the user to call upon the destructor before freeing the memory!

class FrameAllocator
{
	friend MemoryManager;

private:
    void* m_pStart;
	void* m_pEnd;
	void* m_pCurrent;
private:
	static SpinLock m_InstanceLock;
    static std::unordered_map<std::thread::id, FrameAllocator*> s_FrameAllocatorMap;
public:
    ~FrameAllocator();

    template<class T, typename... Args>
    T* Allocate(Args&& ... args);
    template<class T, typename... Args>
    T* AllocateAligned(size_t alignment, Args&& ... args);
    template<class T>
    T* AllocateArray(size_t count, size_t alignment = 1);
    
    void Reset();
    
    inline size_t GetAllocatedMemory() const
    {
        return (size_t)m_pEnd - (size_t)m_pCurrent;
    }
    
    
    inline size_t GetTotalMemory() const
    {
        return (size_t)m_pEnd - (size_t)m_pStart;
    }
private:
	FrameAllocator(size_t size);
	FrameAllocator(void* start, void* end);

    void* AllocateMem(size_t size, size_t alignment);
public:
	static FrameAllocator& GetInstance()
	{
		std::lock_guard<SpinLock> lock(m_InstanceLock);
		std::thread::id id = std::this_thread::get_id();
		auto search = s_FrameAllocatorMap.find(id);
		if (search != s_FrameAllocatorMap.end())
		{
			return *(search->second);
		}
		FrameAllocator* allocator = new FrameAllocator(1024 * 1024);
		s_FrameAllocatorMap.insert({ id, allocator });
		return *allocator;
	}

private:
	static void Release()
	{
		for (auto& it : s_FrameAllocatorMap)
		{
			delete it.second;
		}
	}
};


template<class T, typename ... Args>
inline T* FrameAllocator::Allocate(Args&&... args)
{
   return new(AllocateMem(sizeof(T), 1)) T(std::forward<Args>(args) ...);
}


template<class T, typename ... Args>
inline T* FrameAllocator::AllocateAligned(size_t alignment, Args&&... args)
{
   return new(AllocateMem(sizeof(T), alignment)) T(std::forward<Args>(args) ...);
}


template<class T>
inline T* FrameAllocator::AllocateArray(size_t count, size_t alignment)
{
    size_t arrSize = sizeof(T) * count;
	return new(AllocateMem(arrSize, alignment)) T[count];;
}
