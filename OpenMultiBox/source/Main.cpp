#include <iostream>
#include <fstream>

#include "EventLoop.h"
#include "WinApiUtil.h"
#include "Window.h"
#include "WindowGroup.h"
#include "Json.h"

struct Settings
{
	std::string ExecutablePath;
	std::string WindowTitle;
	int InstanceCount;
};

Settings LoadSettings()
{
	std::ifstream fileStream("settings.json");

	try
	{
		auto j = nlohmann::json::parse(fileStream);

		Settings settings;
		settings.ExecutablePath = j["executablePath"];
		settings.WindowTitle = j["windowTitle"];
		settings.InstanceCount = j["instanceCount"];

		return settings;
	}
	catch (nlohmann::json::parse_error& e)
	{
		std::cout << "Error parsing json: " << e.what() << std::endl
			<< "exception id: " << e.id << std::endl
			<< "byte position of error: " << e.byte << std::endl;

		throw e;
	}
}

int main()
{
	const int SleepInterval = 100;

	const auto& settings = LoadSettings();

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

	int windowCount = 5;
	std::string path = "G:/World of Warcraft/_retail_/Wow.exe";
	std::string title = "World of Warcraft";

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

	dispatchAction([&group]()
	{ 
		group.SetupKeyboardBroadcastHook();
		//group.SetupMouseBroadcastHook();
	});

	while (!eventLoop.IsStopped())
	{
		try
		{
			group.RearrangeIfPrimaryChanged();
		}
		catch (const std::exception& ex)
		{
			break;
		}

		Sleep(SleepInterval);
	}

	std::cout << "Done" << std::endl;

	group.RemoveHooks();

	eventLoop.Stop();

	return 0;
}
