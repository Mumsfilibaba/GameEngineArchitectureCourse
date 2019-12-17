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

#define MAX_THREADS 8

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

    
	inline bool IsFinished() const
	{
		return m_CurrentFence <= m_FinishedFence.load();
	}
    
    
    inline bool ShouldRunWorker() const
    {
        return m_RunWorkers;
    }
private:
	void Poll();
	bool Poptask(std::function<void()>& task);
private:
    bool m_RunWorkers;
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

