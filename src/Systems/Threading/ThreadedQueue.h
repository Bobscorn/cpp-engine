#pragma once

#include <mutex>
#include <queue>
#include <condition_variable>
#include <chrono>

namespace Threading
{
	template<class T>
	class ThreadedQueue
	{
		std::queue<T> _queue;
		std::mutex _mutex;
		std::condition_variable _waitCV;
	public:
		ThreadedQueue() = default;
		ThreadedQueue(ThreadedQueue&& other) = default;
		ThreadedQueue(const ThreadedQueue& other) = delete;
		~ThreadedQueue() = default;

		ThreadedQueue& operator=(const ThreadedQueue& other) = delete;
		ThreadedQueue& operator=(ThreadedQueue&& other) = delete;

		T pop()
		{
			std::unique_lock<std::mutex> lock(_mutex);

			while (_queue.empty())
			{
				_waitCV.wait(lock);
			}

			auto p = std::move(_queue.front());
			_queue.pop();
			return p;
		}

		bool try_pop(T& outVal)
		{
			std::unique_lock<std::mutex> lock(_mutex);

			if (_queue.empty())
			{
				return false;
			}

			outVal = std::move(_queue.front());
			_queue.pop();
			return true;
		}

		template<class rep, class period>
		bool try_pop(T& outVal, const std::chrono::duration<rep, period>& dur)
		{
			std::unique_lock<std::mutex> lock(_mutex);

			if (_queue.empty())
			{
				_waitCV.wait_for(lock, dur);

				if (_queue.empty())
					return false;
			}

			outVal = _queue.front();
			_queue.pop();
			return true;
		}

		void push(T val)
		{
			std::unique_lock<std::mutex> lock(_mutex);

			if constexpr (std::is_move_constructible_v<T>)
				_queue.push(std::move(val));
			else
				_queue.push(val);
			
			lock.unlock();
			_waitCV.notify_one();
		}
	};
}