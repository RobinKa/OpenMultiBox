#include "WindowGroup.h"

#include "WinApiUtil.h"

#include <iostream>

void omb::WindowGroup::AddWindow(Window* window)
{
	windows.push_back(window);

	if (!primaryWindow)
	{
		primaryWindow = window;
	}
}

void omb::WindowGroup::Rearrange()
{
	auto screenSize = omb::GetMainScreenSize();
	auto windowCount = windows.size();

	std::pair<int, int> secondarySize = std::make_pair((int)(screenSize.first / 5.0), (int)(screenSize.second / 5.0));

	int primaryWidth = screenSize.first - secondarySize.first;
	int primaryHeight = (int)(screenSize.second * ((double)primaryWidth / screenSize.first));

	int secondaryIndex = 0;

	const auto focusedWindow = GetFocusedWindow();
	if (focusedWindow)
	{
		primaryWindow = focusedWindow;
	}

	for (auto window : windows)
	{
		if (window == primaryWindow)
		{
			window->SetRect(0, 0, primaryWidth, primaryHeight);
		}
		else
		{
			window->SetRect(primaryWidth, secondaryIndex * secondarySize.second, secondarySize.first, secondarySize.second);
			secondaryIndex++;
		}
	}
}

void omb::WindowGroup::RearrangeIfPrimaryChanged()
{
	if (GetFocusedWindow() != primaryWindow)
	{
		Rearrange();
	}
}

omb::Window* omb::WindowGroup::GetFocusedWindow() const
{
	for (auto window : windows)
	{
		if (omb::IsFocusedWindow(window->GetHandle()))
		{
			return window;
		}
	}

	return nullptr;
}

static omb::WindowGroup* WindowGroupInstance = nullptr;

void omb::WindowGroup::SetupKeyboardBroadcastHook()
{
	WindowGroupInstance = this;

	auto cb = [](int nCode, WPARAM wParam, LPARAM lParam) -> LRESULT
	{
		if (wParam == WM_KEYDOWN && nCode >= 0)
		{
			const KBDLLHOOKSTRUCT* data = (KBDLLHOOKSTRUCT*)lParam;

			// Broadcast key to secondary windows
			for (auto window : WindowGroupInstance->windows)
			{
				if (WindowGroupInstance->primaryWindow != window)
				{
					PostMessage(window->GetHandle(), WM_KEYDOWN, (WPARAM)data->vkCode, NULL);
				}
			}
		}

		return CallNextHookEx(WindowGroupInstance->keyboardHookHandle, nCode, wParam, lParam);
	};

	keyboardHookHandle = SetWindowsHookEx(WH_KEYBOARD_LL, cb, 0, 0);
}