#include <iostream>

#include "EventLoop.h"
#include "WinApiUtil.h"
#include "Window.h"
#include "WindowGroup.h"

int main()
{
	const int SleepInterval = 100;

	std::cout << "Enter number of windows: ";
	std::cin >> windowCount;

	std::string path;
	std::cout << "Enter path to exe: ";
	std::getline(std::cin >> std::ws, path);

	std::string title;
	std::cout << "Enter window title: ";
	std::getline(std::cin >> std::ws, title);

	omb::EventLoop eventLoop;

	// Enqueues an action in the event loop and waits until it is ran.
	auto dispatchAction = [&eventLoop](std::function<void()> action)
	{
		bool actionDone = false;
		eventLoop.EnqueueAction([&action, &actionDone]()
		{
			action();
			actionDone = true;
		});

		while (!actionDone)
		{
			Sleep(SleepInterval);
		}
	};

	eventLoop.Run();

	std::vector<PROCESS_INFORMATION> procInfos;

	dispatchAction([&procInfos, path, windowCount]()
	{
		for (int i = 0; i < windowCount; i++)
		{
			procInfos.push_back(omb::Launch(path));
		}
	});

	// Wait until all windows are opened and thus found
	std::vector<omb::Window> windows;

	while (procInfos.size() > 0)
	{
		dispatchAction([&windows, &procInfos, title]()
		{
			for (size_t i = procInfos.size(); i --> 0;)
			{
				try
				{
					HWND windowHandle = omb::FindProcessWindowHandle(procInfos[i].dwProcessId, title);
					windows.push_back(omb::Window(windowHandle));
					procInfos.erase(procInfos.begin() + i);
				}
				catch (const std::exception& ex)
				{
				}
			}
		});
	}

	std::cout << "Loaded all windows" << std::endl;

	omb::WindowGroup group;
	for (auto& window : windows)
	{
		group.AddWindow(&window);
	}

	group.Rearrange();

	dispatchAction([&group]() { group.SetupKeyboardBroadcastHook(); });

	while (!eventLoop.IsStopped())
	{
		group.RearrangeIfPrimaryChanged();
		Sleep(SleepInterval);
	}

	eventLoop.Stop();

	return 0;
}
