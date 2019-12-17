#pragma once
#include <atomic>
#include "MemoryManager.h"

//Objects allocated through this allocator will never have their destruct called from it. Therefore it is up to the user to call upon the destructor before freeing the memory!
class StackAllocator
{
public:
	StackAllocator(size_t size);
    ~StackAllocator();

#ifdef SHOW_ALLOCATIONS_DEBUG
	void* AllocateMemory(const std::string& tag, size_t size, size_t alignment);
#else
	void* AllocateMemory(size_t size, size_t alignment);
#endif
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
	void* m_pStart;
	void* m_pEnd;
	void* m_pCurrent;
	size_t m_Used;
	const size_t m_Size;
public:
	static StackAllocator& GetInstance(size_t size = 1024 * 1024 * 64) // = 64MB
	{
		thread_local static StackAllocator instance(size);
		return instance;
	}

	static size_t GetTotalAvailableMemory()
	{
		return s_TotalAllocated;
	}

	static size_t GetTotalUsedMemory()
	{
		return s_TotalUsed;
	}
private:
	static std::atomic_size_t s_TotalAllocated;
	static std::atomic_size_t s_TotalUsed;
};

namespace Helpers
{
	struct StackDummy {};
}

#ifdef SHOW_ALLOCATIONS_DEBUG
inline void* operator new(size_t size, size_t alignment, Helpers::StackDummy, const std::string& tag)
{
	return StackAllocator::GetInstance().AllocateMemory(tag, size, alignment);
}

#define stack_allocate(size, alignment, tag) StackAllocator::GetInstance().AllocateMemory(tag, size, alignment)
#define stack_new(tag)			new(1, Helpers::StackDummy(), tag)
#define stack_delete(object)	{ using T = std::remove_pointer< std::remove_reference<decltype(object)>::type >::type; object->~T(); }
#define stack_reset				StackAllocator::GetInstance().Reset
#else
inline void* operator new(size_t size, size_t alignment, Helpers::StackDummy)
{
	return StackAllocator::GetInstance().AllocateMemory(size, alignment);
}

#define stack_new				new(1, Helpers::StackDummy())
#define stack_delete(object)	{ using T = std::remove_pointer< std::remove_reference<decltype(object)>::type >::type; object->~T(); }
#define stack_reset				StackAllocator::GetInstance().Reset
#endif