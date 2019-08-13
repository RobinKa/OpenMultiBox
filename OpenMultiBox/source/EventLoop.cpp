#include "EventLoop.h"

#include <thread>

void omb::EventLoop::Run()
{
	thread = std::thread([this]()
	{
		MSG msg;
		ZeroMemory(&msg, sizeof(MSG));

		while (WM_QUIT != msg.message && !stop)
		{
			while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) > 0)
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}

			std::lock_guard<std::mutex> guard(actionQueueMutex);
			while (actionQueue.size() > 0)
			{
				auto action = actionQueue.front();
				action();
				actionQueue.pop();
			}
		}
	});
}

void omb::EventLoop::Stop()
{
	stop = true;
	thread.join();
}

void omb::EventLoop::EnqueueAction(std::function<void()> action)
{
	std::lock_guard<std::mutex> guard(actionQueueMutex);
	actionQueue.push(action);
}