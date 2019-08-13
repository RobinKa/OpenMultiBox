#include <iostream>
#include <fstream>
#include <filesystem>

#include "EventLoop.h"
#include "WinApiUtil.h"
#include "Window.h"
#include "WindowGroup.h"
#include "Json.h"

namespace fs = std::filesystem;

struct Settings
{
	fs::path WowPath;
	std::string WindowTitle;
	int InstanceCount;
	std::string CopyFromAccount;
	std::string CopyFromCharacter;
	std::vector<std::string> CopyToAccounts;
	std::vector<std::string> CopyToCharacters;
};

Settings LoadSettings()
{
	std::ifstream fileStream("settings.json");

	try
	{
		auto j = nlohmann::json::parse(fileStream);

		Settings settings;
		settings.WowPath = fs::path(std::string(j["wowPath"]));
		settings.WindowTitle = j["windowTitle"];
		settings.InstanceCount = j["instanceCount"];
		settings.CopyFromAccount = j["copyFromAccount"];
		settings.CopyFromCharacter = j["copyFromCharacter"];
		settings.CopyToAccounts = j["copyToAccounts"].get<std::vector<std::string>>();
		settings.CopyToCharacters = j["copyToCharacters"].get<std::vector<std::string>>();

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

	eventLoop.Run();

	std::vector<PROCESS_INFORMATION> procInfos;

	dispatchAction([&procInfos, &settings]()
	{
		auto path = settings.WowPath / "Wow.exe";
		for (int i = 0; i < settings.InstanceCount; i++)
		{
			procInfos.push_back(omb::Launch(path.string()));
		}
	});

	// Wait until all windows are opened and thus found
	std::vector<omb::Window> windows;

	while (procInfos.size() > 0)
	{
		dispatchAction([&windows, &procInfos, &settings]()
		{
			for (size_t i = procInfos.size(); i --> 0;)
			{
				try
				{
					HWND windowHandle = omb::FindProcessWindowHandle(procInfos[i].dwProcessId, settings.WindowTitle);
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

	group.AddHotkeyCallback(VK_F8, [&group, &eventLoop]()
	{
		std::cout << "Toggle stay on top" << std::endl;

		eventLoop.EnqueueAction([&group]()
		{
			group.SetStayOnTop(!group.GetStayOnTop());
		});
	});

	group.AddHotkeyCallback(VK_F9, [&settings]()
	{
		std::cout << "Copying WTF config" << std::endl;

		std::vector<fs::path> filesToCopy =
		{
			"macros-cache.txt",
			"bindings-cache.wtf",
			"config-cache.wtf"
		};

		const auto& wtfPath = settings.WowPath / "WTF" / "Account";

		// Copy account files
		const auto& fromPath = wtfPath / settings.CopyFromAccount;
		for (const auto& copyToAccount : settings.CopyToAccounts)
		{
			const auto& toPath = wtfPath / copyToAccount;

			for (const auto& fileToCopy : filesToCopy)
			{
				const auto& fromFilePath = fromPath / fileToCopy;
				const auto& toFilePath = toPath / fileToCopy;

				std::cout << fromFilePath << " => " << toFilePath << std::endl;

				fs::copy_file(fromFilePath, toFilePath, fs::copy_options::overwrite_existing);
			}
		}

		// Copy character files
		const auto& characterFromPath = wtfPath / settings.CopyFromCharacter / "SavedVariables";
		const auto& characterFromAddonsPath = wtfPath / settings.CopyFromCharacter / "AddOns.txt";
		for (const auto& copyToCharacter : settings.CopyToCharacters)
		{
			const auto& characterToPath = wtfPath / copyToCharacter / "SavedVariables";
			const auto& characterToAddonsPath = wtfPath / copyToCharacter / "AddOns.txt";

			std::cout << characterToPath << " => " << characterToAddonsPath << std::endl;

			fs::copy_file(characterFromAddonsPath, characterToAddonsPath, fs::copy_options::overwrite_existing);

			std::cout << characterFromPath << " => " << characterToPath << std::endl;

			fs::copy(characterFromPath, characterToPath, fs::copy_options::overwrite_existing | fs::copy_options::recursive);
		}
	});

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
