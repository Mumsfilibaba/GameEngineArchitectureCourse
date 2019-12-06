#include "TaskManager.h"
#include <iostream>
#include <algorithm>


void TaskManager::TaskThread()
{
	while (TaskManager::Get().ShouldRunWorker())
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
    
    ThreadSafePrintf("Shutting down worker\n");
}


TaskManager::TaskManager()
    : m_RunWorkers(true),
    m_Tasks(),
    m_Mutex(),
    m_WakeCondition(),
    m_FinishedFence(0),
    m_CurrentFence(0),
    m_QueueLock()
{
	uint32_t numThreads = std::max(1U, std::thread::hardware_concurrency());
	ThreadSafePrintf("TaskManager: Starting up %u threads\n", numThreads);

    //Yes run all workers
    m_RunWorkers = true;
    
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
    
    //No stop run all workers
    m_RunWorkers = false;
    
    //Notify all workers to check
    m_WakeCondition.notify_all();
    
    //Then we wait
	Wait();
    
	ThreadSafePrintf("TaskManager: All tasks are finished\n");
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


bool TaskManager::Poptask(std::function<void()>& task)
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
