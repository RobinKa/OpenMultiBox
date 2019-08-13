#pragma once

#include <queue>
#include <string>
#include <functional>
#include <mutex>
#include <Windows.h>

namespace omb
{
	class EventLoop
	{
	public:
		void Run();
		void Stop();

		void EnqueueAction(std::function<void()> action);

		bool IsStopped() const;

	private:
		std::queue<std::function<void()>> actionQueue;
		std::mutex actionQueueMutex;
		std::thread thread;
		bool stop = false;
		bool stopped = true;
	};
}