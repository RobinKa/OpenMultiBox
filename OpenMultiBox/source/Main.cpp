#include <iostream>

#include "EventLoop.h"
#include "WinApiUtil.h"
#include "Window.h"
#include "WindowGroup.h"

int main()
{
	int windowCount;
	std::cout << "Enter number of windows: ";
	std::cin >> windowCount;

	std::string path;
	std::cout << "Enter path to exe: ";
	std::getline(std::cin >> std::ws, path);

	std::string title;
	std::cout << "Enter window title: ";
	std::getline(std::cin >> std::ws, title);

	omb::EventLoop eventLoop;
	eventLoop.Run();

	bool actionDone = false;

	std::vector<PROCESS_INFORMATION> procInfos;

	eventLoop.EnqueueAction([&actionDone, &procInfos, path, windowCount]()
	{
		for (int i = 0; i < windowCount; i++)
		{
			procInfos.push_back(omb::Launch(path));
		}

		actionDone = true;
	});

	while (!actionDone) Sleep(200);
	actionDone = false;

	// Wait until all windows are opened and thus found
	std::vector<omb::Window> windows;

	while (procInfos.size() > 0)
	{
		eventLoop.EnqueueAction([&actionDone, &windows, &procInfos, title]()
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

			actionDone = true;
		});

		while (!actionDone) Sleep(200);
		actionDone = false;
	}

	std::cout << "Loaded all windows" << std::endl;

	omb::WindowGroup group;
	for (auto& window : windows)
	{
		group.AddWindow(&window);
	}

	group.Rearrange();

	for (int i = 0; i < 100; i++)
	{
		group.RearrangeIfPrimaryChanged();
		Sleep(500);
	}

	eventLoop.Stop();

	return 0;
}
