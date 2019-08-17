#include "WindowGroup.h"

#include "WinApiUtil.h"

#include <iostream>
#include <thread>

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

	if (windowSwitching)
	{
		const auto focusedWindow = GetFocusedWindow();
		if (focusedWindow)
		{
			primaryWindow = focusedWindow;
		}
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

	for (const auto& callback : rearrangeCallbacks)
	{
		callback();
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

inline LPARAM GetKeyEventParameters(WPARAM key)
{
	if (key == WM_KEYDOWN || key == WM_SYSKEYDOWN)
	{
		// https://docs.microsoft.com/en-us/windows/win32/inputdev/wm-keydown
		return 1ul;
	}
	else if (key == WM_KEYUP || key == WM_SYSKEYUP)
	{
		// https://docs.microsoft.com/en-us/windows/win32/inputdev/wm-keyup
		return (1ul << 31ul) | (1ul << 30ul) | 1ul;
	}

	throw std::exception("Invalid key passed to GetKeyEventParameters");
}

inline bool IsMovementKey(DWORD key)
{
	return key == 'w' || key == 'a' || key == 's' || key == 'd' || key == 'q' || key == 'e' ||
		key == 'W' || key == 'A' || key == 'S' || key == 'D' || key == 'Q' || key == 'E';
}

void omb::WindowGroup::SetupKeyboardBroadcastHook()
{
	WindowGroupInstance = this;

	auto cb = [](int nCode, WPARAM wParam, LPARAM lParam) -> LRESULT
	{
		if ((wParam == WM_KEYDOWN || wParam == WM_KEYUP || wParam == WM_SYSKEYDOWN || wParam == WM_SYSKEYUP) && nCode >= 0)
		{
			const KBDLLHOOKSTRUCT* data = (KBDLLHOOKSTRUCT*)lParam;

			const auto& hotkeyCallbacks = WindowGroupInstance->GetHotkeyCallbacks();

			if (wParam == WM_KEYUP && hotkeyCallbacks.contains(data->vkCode))
			{
				hotkeyCallbacks.at(data->vkCode)();
			}
			else if (WindowGroupInstance->GetBroadcast() && (WindowGroupInstance->GetBroadcastMovement() || !IsMovementKey(data->vkCode)))
			{
				auto primaryWindow = WindowGroupInstance->GetPrimaryWindow();

				// Broadcast key to secondary windows
				for (auto window : WindowGroupInstance->GetWindows())
				{
					if (window != primaryWindow)
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

			HWND cursorWindowHandle = WindowFromPoint(data->pt);

			std::cout << "Cursor window handle: " << cursorWindowHandle << " | Primary handles: " <<
				WindowGroupInstance->primaryWindow->GetHandles()[0] << ", " <<
				WindowGroupInstance->primaryWindow->GetHandles()[1] << std::endl;

			// TODO: Broadcast click
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

void omb::WindowGroup::AddRearrangeCallback(std::function<void()> callback)
{
	rearrangeCallbacks.push_back(callback);
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

bool omb::WindowGroup::GetBroadcastMovement() const
{
	return broadcastMovement;
}

void omb::WindowGroup::SetBroadcastMovement(bool b)
{
	broadcastMovement = b;
}

bool omb::WindowGroup::GetBroadcast() const
{
	return broadcast;
}

void omb::WindowGroup::SetBroadcast(bool b)
{
	broadcast = b;
}

const std::map<DWORD, std::function<void()>>& omb::WindowGroup::GetHotkeyCallbacks()
{
	return hotkeyCallbacks;
}

const omb::Window* omb::WindowGroup::GetPrimaryWindow() const
{
	return primaryWindow;
}

const std::vector<omb::Window*>& omb::WindowGroup::GetWindows() const
{
	return windows;
}

void omb::WindowGroup::LeftClick(int delayMs)
{
	if (!clicking)
	{
		clicking = true;
		windowSwitching = false;

		std::thread t([this, delayMs]()
		{
			omb::LeftClickWindows(GetWindows(), delayMs);
			windowSwitching = true;
			clicking = false;
		});

		t.detach();
	}
}