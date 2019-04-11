#pragma once
#include <thread>
#include <vector>
#include <list>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <atomic>
#include <functional>

class ThreadGrid
{
	int numThreads;

	std::vector<std::thread> threads;
	std::vector<std::mutex> threadIsActive;

	std::atomic_bool terminate{ false };

	std::function<void(int, int)> task;
	std::mutex taskLock;
	bool hasTask{ false };
	std::condition_variable taskAwailableCond;

	std::mutex numActiveThreadsLock;
	int numActiveThreads;
	std::condition_variable hasActiveThreadsCond;
	
public:
	ThreadGrid(int n)
		: numThreads(n)
		, threads(n)
		, threadIsActive(n)
	{
		for (int i = 0; i < n; ++i)
		{
			threads[i] = std::thread(&ThreadGrid::Thread, this, i);
		}
	}

	~ThreadGrid()
	{
		terminate = true;
		{
			std::lock_guard<std::mutex> m(taskLock);
			taskAwailableCond.notify_all();
		}

		for (auto& thread : threads)
		{
			if (thread.joinable())
				thread.join();
		}
	}

	void GridRun(std::function<void(int, int)>&& item)
	{
		SetAllThreadsActive();

		SetTask(std::move(item));

		WaitAllThreadsComplete();

		ClearTask();
	}

private:
	void Thread(int threadIdx)
	{
		while (!terminate)
		{
			std::function<void(int, int)> item;

			{
				std::unique_lock<std::mutex> m(taskLock);
				taskAwailableCond.wait(m, [&]{return hasTask || terminate; });
				if (!hasTask)
					continue;
				item = task;
			}

			item(threadIdx, numThreads);
			SetThreadComplete();
		}
	}

	void SetTask(std::function<void(int, int)>&& item)
	{
		std::lock_guard<std::mutex> m(taskLock);
		task = std::move(item);
		hasTask = true;
		taskAwailableCond.notify_all();
	}

	// main purpose is to de-activate the initial closure we capture 
	void ClearTask()
	{
		std::lock_guard<std::mutex> m(taskLock);
		task = std::function<void(int, int)>();
		hasTask = false;
	}

	void SetAllThreadsActive()
	{
		std::lock_guard<std::mutex> m(numActiveThreadsLock);
		numActiveThreads = numThreads;
	}

	void SetThreadComplete()
	{
		std::unique_lock<std::mutex> m(numActiveThreadsLock);
		if (--numActiveThreads == 0)
			hasActiveThreadsCond.notify_all();
	}

	void WaitAllThreadsComplete()
	{
		std::unique_lock<std::mutex> m(numActiveThreadsLock);
		while (numActiveThreads > 0)
			hasActiveThreadsCond.wait(m);
	}
};