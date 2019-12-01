#include "TaskManager.h"
#include <iostream>
#include <algorithm>


void TaskManager::TaskThread()
{
	while (true)
	{
		Task task = {};
		if (TaskManager::Get().Poptask(&task))
		{
			//ThreadSafePrint("Took Task");

			task.TaskFunc();
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
{
	unsigned int numThreads = std::max(1U, std::thread::hardware_concurrency());
	std::cout << "Starting up " << numThreads << "threads" << std::endl;

	//Startup all the threads
	for (int i = 0; i < numThreads; i++)
	{
		std::thread worker(TaskManager::TaskThread);
		worker.detach();
	}
}


void TaskManager::Execute(const Task& pTask)
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


const bool TaskManager::Poptask(Task* pTask)
{
	std::lock_guard<SpinLock> lock(m_QueueLock);
	if (!m_Tasks.empty())
	{
		*pTask = m_Tasks.front();
		m_Tasks.pop();
		return true;
	}

	return false;
}