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
			window->SetRect(0, 0, primaryWidth, primaryHeight, stayOnTop);
		}
		else
		{
			window->SetRect(primaryWidth, secondaryIndex * secondarySize.second, secondarySize.first, secondarySize.second, stayOnTop);
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
		if (window->IsFocused())
		{
			return window;
		}
	}

	return nullptr;
}

static omb::WindowGroup* WindowGroupInstance = nullptr;

LPARAM GetKeyEventParameters(WPARAM key)
{
	if (key == WM_KEYDOWN)
	{
		// https://docs.microsoft.com/en-us/windows/win32/inputdev/wm-keydown
		return 1ul;
	}
	else if (key == WM_KEYUP)
	{
		// https://docs.microsoft.com/en-us/windows/win32/inputdev/wm-keyup
		return (1ul << 31ul) | (1ul << 30ul) | 1ul;
	}

	throw std::exception("Invalid key passed to GetKeyEventParameters");
}

void omb::WindowGroup::SetupKeyboardBroadcastHook()
{
	WindowGroupInstance = this;

	auto cb = [](int nCode, WPARAM wParam, LPARAM lParam) -> LRESULT
	{
		if ((wParam == WM_KEYDOWN || wParam == WM_KEYUP) && nCode >= 0)
		{
			const KBDLLHOOKSTRUCT* data = (KBDLLHOOKSTRUCT*)lParam;

			if (data->vkCode == VK_F10 || data->vkCode == VK_F11)
			{
				WindowGroupInstance->broadcast = data->vkCode == VK_F10;
				std::cout << "Broadcast keyboard: " << WindowGroupInstance->broadcast << std::endl;
			}
			else if (wParam == WM_KEYUP && WindowGroupInstance->hotkeyCallbacks.contains(data->vkCode))
			{
				WindowGroupInstance->hotkeyCallbacks[data->vkCode]();
			}
			else if (WindowGroupInstance->broadcast)
			{
				// Broadcast key to secondary windows
				for (auto window : WindowGroupInstance->windows)
				{
					if (WindowGroupInstance->primaryWindow != window)
					{
						for (auto windowHandle : window->GetHandles())
						{
							PostMessage(windowHandle, (UINT)wParam, (WPARAM)data->vkCode, GetKeyEventParameters(wParam));
						}
					}
				}
			}
		}

		return CallNextHookEx(WindowGroupInstance->keyboardHookHandle, nCode, wParam, lParam);
	};

	keyboardHookHandle = SetWindowsHookEx(WH_KEYBOARD_LL, cb, 0, 0);
}

void omb::WindowGroup::SetupMouseBroadcastHook()
{
	WindowGroupInstance = this;

	auto cb = [](int nCode, WPARAM wParam, LPARAM lParam) -> LRESULT
	{
		if ((wParam == WM_LBUTTONDOWN || wParam == WM_LBUTTONUP) && nCode >= 0)
		{
			const MSLLHOOKSTRUCT* data = (MSLLHOOKSTRUCT*)lParam;

			// Broadcast key to secondary windows
			for (auto window : WindowGroupInstance->windows)
			{
				if (WindowGroupInstance->primaryWindow != window)
				{
					// TODO: Get window at cursor and use that one for transforming from
					POINT windowPoint = omb::TransformWindowPoint(WindowGroupInstance->primaryWindow->GetHandles()[0], window->GetHandles()[0], data->pt);
					PostMessage(window->GetHandles()[0], (UINT)wParam, MK_LBUTTON, MAKELPARAM(windowPoint.x, windowPoint.y));
				}
			}
		}

		return CallNextHookEx(WindowGroupInstance->keyboardHookHandle, nCode, wParam, lParam);
	};

	mouseHookHandle = SetWindowsHookEx(WH_MOUSE_LL, cb, 0, 0);
}

void omb::WindowGroup::RemoveHooks()
{
	if (keyboardHookHandle)
	{
		UnhookWindowsHookEx(keyboardHookHandle);
		keyboardHookHandle = NULL;
	}

	if (mouseHookHandle)
	{
		UnhookWindowsHookEx(mouseHookHandle);
		mouseHookHandle = NULL;
	}
}

void omb::WindowGroup::AddHotkeyCallback(DWORD key, std::function<void()> callback)
{
	hotkeyCallbacks.emplace(key, callback);
}

void omb::WindowGroup::SetStayOnTop(bool b)
{
	bool changed = stayOnTop == b;
	stayOnTop = b;
	if (changed)
	{
		Rearrange();
	}
}

bool omb::WindowGroup::GetStayOnTop() const
{
	return stayOnTop;
}