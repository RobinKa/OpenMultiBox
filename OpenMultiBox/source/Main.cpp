#include <iostream>
#include <fstream>
#include <filesystem>
#include <chrono>

#include "EventLoop.h"
#include "WinApiUtil.h"
#include "Window.h"
#include "WindowGroup.h"
#include "Json.h"
#include "UserInterface.h"

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

int main(int argc, char** argv)
{
	omb::UserInterface ui;
	ui.Start();


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
	std::vector<int> idWindowIds;
	std::vector<int> cursorWindowIds;

	while (procInfos.size() > 0)
	{
		dispatchAction([&windows, &procInfos, &settings, &idWindowIds, &cursorWindowIds, &ui]()
		{
			for (size_t i = procInfos.size(); i --> 0;)
			{
				try
				{
					const auto& windowHandles = omb::FindProcessWindowHandles(procInfos[i].dwProcessId, settings.WindowTitle);
					windows.push_back(omb::Window(windowHandles));
					procInfos.erase(procInfos.begin() + i);
					idWindowIds.push_back(ui.CreateIdWindow());
					cursorWindowIds.push_back(ui.CreateCursorWindow());
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

	std::thread cursorThread([&ui, &eventLoop, &cursorWindowIds, &group]()
	{
		try
		{
			while (!eventLoop.IsStopped())
			{
				POINT cursorPos;
				if (GetCursorPos(&cursorPos))
				{
					HWND cursorWindowHandle = WindowFromPoint(cursorPos);

					if (cursorWindowHandle)
					{
						const auto& windows = group.GetWindows();
						for (int i = 0; i < cursorWindowIds.size(); i++)
						{
							const auto& virtualCursorPos = omb::TransformWindowPoint(cursorWindowHandle, windows[i]->GetHandles()[0], cursorPos);
							ui.SetCursorWindowPosition(cursorWindowIds[i], virtualCursorPos.x, virtualCursorPos.y, !windows[i]->IsFocused());
						}
					}
				}

				std::this_thread::sleep_for(std::chrono::milliseconds(10));
			}
		}
		catch (const std::exception& ex)
		{
			std::cout << "Cursor thread exception: " << ex.what() << std::endl;
		}
	});

	group.AddRearrangeCallback([&ui, &idWindowIds, &windows, &group]()
	{
		for (int i = 0; i < idWindowIds.size(); i++)
		{
			const auto& pos = windows[i].GetPosition();
			ui.SetIdWindowPosition(idWindowIds[i], pos.first, pos.second);
		}

		const auto& mainPos = group.GetPrimaryWindow()->GetPosition();
		const auto& mainSize = group.GetPrimaryWindow()->GetSize();
		ui.SetMainWindowPosition(mainPos.first + mainSize.first / 2, mainPos.second);
	});

	group.AddHotkeyCallback(VK_OEM_1, [&group]()
	{
		group.LeftClick(35);
	});

	group.AddHotkeyCallback(VK_F7, [&group, &eventLoop, &ui]()
	{
		eventLoop.EnqueueAction([&group, &ui]()
		{
			group.SetBroadcastMovement(!group.GetBroadcastMovement());
			ui.SetMovementBroadcast(group.GetBroadcastMovement());
			std::cout << "Broadcast movement: " << group.GetBroadcastMovement() << std::endl;
		});
	});

	group.AddHotkeyCallback(VK_F8, [&group, &eventLoop, &ui]()
	{
		eventLoop.EnqueueAction([&group, &ui]()
		{
			group.SetStayOnTop(!group.GetStayOnTop());
			ui.SetStayOnTop(group.GetStayOnTop());
			std::cout << "Stay on top: " << group.GetStayOnTop() << std::endl;
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

	group.AddHotkeyCallback(VK_F10, [&group, &eventLoop, &ui]()
	{
		eventLoop.EnqueueAction([&group, &ui]()
		{
			group.SetBroadcast(!group.GetBroadcast());
			ui.SetBroadcast(group.GetBroadcast());
			std::cout << "Broadcast: " << group.GetBroadcast() << std::endl;
		});
	});

	group.Rearrange();

	dispatchAction([&group]()
	{ 
		group.SetupKeyboardBroadcastHook();
	});

	/*std::thread t([&group, &eventLoop]()
	{
		group.SetupMouseBroadcastHook();

		while (!eventLoop.IsStopped())
		{
			MSG msg;
			ZeroMemory(&msg, sizeof(MSG));

			while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) > 0)
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	});*/

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

	ui.Stop();

	group.RemoveHooks();

	eventLoop.Stop();

	cursorThread.join();

	//t.join();

	return 0;
}
