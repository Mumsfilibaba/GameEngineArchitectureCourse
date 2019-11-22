#include "PoolAllocator.h"

std::atomic_int32_t PoolAllocatorBase::s_TotalAllocated = 0;
std::atomic_int32_t PoolAllocatorBase::s_TotalUsed = 0;