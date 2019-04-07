#pragma once
#include <thread>
#include <vector>
#include <list>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <atomic>

class Runnable 
{
public:
	virtual ~Runnable() {}
	virtual void Run() = 0;
};

class ThreadPool
{
	std::vector<std::thread> threads;
	std::list<std::unique_ptr<Runnable>> queue;
	std::mutex queueLock;
	std::condition_variable queueCond;

	std::atomic_bool terminate{ false };

public:
	ThreadPool(int n) 
		: threads(n)
	{
		for (int i = 0; i < n; ++i)
		{
			threads[i] = std::thread(&ThreadPool::Thread, this);
		}
	}

	~ThreadPool()
	{
		terminate = true;
		{
			std::lock_guard<std::mutex> m(queueLock);
			queueCond.notify_all();

		}
		for (auto& thread : threads)
		{
			if (thread.joinable())
				thread.join();
		}
	}

	void Enqueue(std::unique_ptr<Runnable>& item)
	{
		std::lock_guard<std::mutex> m(queueLock);
		queue.push_back(item);
		queueCond.notify_one();
	}

private:
	void Thread()
	{
		while (!terminate)
		{
			std::unique_ptr<Runnable> item;

			{
				std::unique_lock<std::mutex> m(queueLock);
				queueCond.wait(m, [&]{return queue.size() > 0 || terminate; });
				item = std::move(queue.front());
				queue.pop_front();
			}

			if (item)
			{
				item->Run();
			}
		}
	}
};