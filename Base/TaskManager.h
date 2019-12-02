#pragma once
#include <iostream>
#include <queue>
#include <mutex>
#include <thread>
#include <cassert>
#include <functional>
#include <condition_variable>
#include "SpinLock.h"
#include "Helpers.h"

class TaskManager
{
public:
	TaskManager(const TaskManager& other) = delete;
	TaskManager(TaskManager&& other) = delete;
	TaskManager& operator=(const TaskManager& other) = delete;
	TaskManager& operator=(TaskManager&& other) = delete;

	TaskManager();
	~TaskManager();

	void Execute(const std::function<void()>& task);
	void Wait();

	inline bool IsFinished()
	{
		return m_CurrentFence <= m_FinishedFence.load();
	}
private:
	void Poll();
	const bool Poptask(std::function<void()>& task);
private:
	std::queue<std::function<void()>> m_Tasks;
	std::mutex m_Mutex;
	std::condition_variable m_WakeCondition;
	std::atomic<uint64_t> m_FinishedFence;
	uint64_t m_CurrentFence;
	SpinLock m_QueueLock;
public:
	inline static TaskManager& Get()
	{
		static TaskManager taskmanager;
		return taskmanager;
	}
private:
	static void TaskThread();
};

