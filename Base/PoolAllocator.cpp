#include "PoolAllocator.h"

std::atomic_size_t PoolAllocatorBase::s_TotalAllocated = 0;
std::atomic_size_t PoolAllocatorBase::s_TotalUsed = 0;