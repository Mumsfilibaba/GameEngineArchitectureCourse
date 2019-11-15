#pragma once
#include <atomic>

class SpinLock
{
public:
	inline void lock() noexcept
	{
		while (m_flag.test_and_set(std::memory_order_acquire));
	}


	inline void unlock() noexcept
	{
		m_flag.clear(std::memory_order_release);
	}


	inline bool try_lock() noexcept
	{
		return !m_flag.test_and_set(std::memory_order_acquire);
	}
private:
	std::atomic_flag m_flag = ATOMIC_FLAG_INIT;
};