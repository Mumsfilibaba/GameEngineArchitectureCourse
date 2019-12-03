#include "TaskManager.h"
#include <iostream>
#include <algorithm>


void TaskManager::TaskThread()
{
	while (true)
	{
		std::function<void()> task;
		if (TaskManager::Get().Poptask(task))
		{
			//ThreadSafePrint("Took Task");

			task();
			TaskManager::Get().m_FinishedFence.fetch_add(1);
		}
		else
		{
			//ThreadSafePrint("No task goin to sleep");

			std::unique_lock<std::mutex> lock(TaskManager::Get().m_Mutex);
			TaskManager::Get().m_WakeCondition.wait(lock);
		}
	}
}


TaskManager::TaskManager()
	: m_CurrentFence(0),
	m_FinishedFence(0),
	m_Mutex(),
	m_QueueLock(),
	m_Tasks(),
	m_WakeCondition()
{
	uint32_t numThreads = std::max(1U, std::thread::hardware_concurrency());
	ThreadSafePrintf("TaskManager: Starting up %u threads\n", numThreads);

	//Startup all the threads
	for (uint32_t i = 0; i < numThreads; i++)
	{
		std::thread worker(TaskManager::TaskThread);
		worker.detach();
	}
}


TaskManager::~TaskManager()
{
	ThreadSafePrintf("TaskManager: Waiting for tasks to finish\n");
	Wait();
}


void TaskManager::Execute(const std::function<void()>& pTask)
{
	m_CurrentFence++;

	std::lock_guard<SpinLock> lock(m_QueueLock);
	m_Tasks.push(pTask);

	m_WakeCondition.notify_one();
}


void TaskManager::Wait()
{
	while (!IsFinished())
	{
		Poll();
	}
}


void TaskManager::Poll()
{
	m_WakeCondition.notify_one();
	std::this_thread::yield();
}


const bool TaskManager::Poptask(std::function<void()>& task)
{
	std::lock_guard<SpinLock> lock(m_QueueLock);
	if (!m_Tasks.empty())
	{
		task = m_Tasks.front();
		m_Tasks.pop();
		return true;
	}

	return false;
}