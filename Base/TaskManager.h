#pragma once
#include <iostream>
#include <queue>
#include <mutex>
#include <thread>
#include <cassert>
#include <condition_variable>
#include "SpinLock.h"
#include "Helpers.h"

struct Task
{
public:
	void (*TaskFunc)(void) = nullptr;
};


class TaskManager
{
public:
	TaskManager(const TaskManager& other) = delete;
	TaskManager(TaskManager&& other) = delete;
	TaskManager& operator=(const TaskManager& other) = delete;
	TaskManager& operator=(TaskManager&& other) = delete;

	TaskManager();
	~TaskManager() = default;

	void Execute(const Task& pTask);
	void Wait();

	inline bool IsFinished()
	{
		return m_CurrentFence <= m_FinishedFence.load();
	}
private:
	void Poll();
	const bool Poptask(Task* pTask);
private:
	std::queue<Task> m_Tasks;
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

