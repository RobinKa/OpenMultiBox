#include "WinApiUtil.h"

#include "Window.h"
#include <thread>

PROCESS_INFORMATION omb::Launch(const std::string& path)
{
	STARTUPINFO startupInfo;
	ZeroMemory(&startupInfo, sizeof(startupInfo));
	startupInfo.cb = sizeof(startupInfo);

	PROCESS_INFORMATION processInfo;
	ZeroMemory(&processInfo, sizeof(processInfo));

	if (!CreateProcess(path.c_str(), "", NULL, NULL, FALSE, 0, NULL, NULL, &startupInfo, &processInfo))
	{
		throw std::exception("Failed to launch process");
	}

	return processInfo;
}

struct EnumWindowCallbackData
{
	std::vector<HWND> WindowHandles;
	std::string WindowTitle;
	DWORD ProcessId;
};

static BOOL CALLBACK FindProcessWindowHandleCallback(HWND hwnd, LPARAM lParam)
{
	EnumWindowCallbackData* data = (EnumWindowCallbackData*)lParam;

	DWORD windowProcessId;
	GetWindowThreadProcessId(hwnd, &windowProcessId);

	char title[256];
	GetWindowText(hwnd, title, 256);

	if (windowProcessId == data->ProcessId && data->WindowTitle == title)
	{
		data->WindowHandles.push_back(hwnd);
	}

	return TRUE;
}

std::vector<HWND> omb::FindProcessWindowHandles(DWORD processId, const std::string& windowTitle)
{
	EnumWindowCallbackData callbackData;
	callbackData.WindowTitle = windowTitle;
	callbackData.ProcessId = processId;

	EnumWindows((WNDENUMPROC)FindProcessWindowHandleCallback, (LPARAM)&callbackData);

	if (callbackData.WindowHandles.size() < 2)
	{
		throw std::exception("Failed to find process window");
	}

	return callbackData.WindowHandles;
}

std::pair<int, int> omb::GetMainScreenSize()
{
	int width = ::GetSystemMetrics(SM_CXSCREEN);
	int height = ::GetSystemMetrics(SM_CYSCREEN);

	return std::make_pair(width, height);
}

bool omb::IsFocusedWindow(HWND hwnd)
{
	return GetForegroundWindow() == hwnd;
}

POINT omb::TransformWindowPoint(HWND originalWindowHandle, HWND targetWindowHandle, POINT point)
{
	RECT originalRect;
	if (!GetClientRect(originalWindowHandle, &originalRect))
	{
		throw std::exception("Failed to get original window rect");
	}

	RECT targetRect;
	if (!GetClientRect(targetWindowHandle, &targetRect))
	{
		throw std::exception("Failed to get target window rect");
	}

	double origWidth = (double)originalRect.right - (double)originalRect.left;
	double origHeight = (double)originalRect.bottom - (double)originalRect.top;
	double targetWidth = (double)targetRect.right - (double)targetRect.left;
	double targetHeight = (double)targetRect.bottom - (double)targetRect.top;

	double relX = (point.x - (double)originalRect.left) / origWidth;
	double relY = (point.y - (double)originalRect.top) / origHeight;

	POINT targetPos;
	targetPos.x = 0;
	targetPos.y = 0;
	ClientToScreen(targetWindowHandle, &targetPos);

	POINT transformed;
	transformed.x = (DWORD)(targetPos.x + relX * targetWidth);
	transformed.y = (DWORD)(targetPos.y + relY * targetHeight);

	return transformed;
}

void omb::LeftClickWindows(const std::vector<Window*>& windows, Window* lastWindow, int delayMs)
{
	auto secondaryWindows = windows;
	Window* primaryWindow = nullptr;

	// Extract primary window since we want to click it last
	auto primaryWindowIt = std::find(std::begin(secondaryWindows), std::end(secondaryWindows), lastWindow);
	if (primaryWindowIt != std::end(secondaryWindows))
	{
		secondaryWindows.erase(primaryWindowIt);
		primaryWindow = lastWindow;
	}

	std::chrono::milliseconds delay(delayMs);

	POINT cursorPos;
	GetCursorPos(&cursorPos);

	HWND cursorWindow;
	cursorWindow = WindowFromPoint(cursorPos);

	for (auto window : secondaryWindows)
	{
		auto windowHandle = window->GetHandles()[0];

		const auto& pos = omb::TransformWindowPoint(cursorWindow, windowHandle, cursorPos);

		mouse_event(MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE, (DWORD)((pos.x / 2560.) * 65535.), (DWORD)((pos.y / 1440.) * 65535.), 0, 0);
		mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
		mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
		std::this_thread::sleep_for(delay);
	}

	if (primaryWindow)
	{
		auto windowHandle = primaryWindow->GetHandles()[0];

		const auto& pos = omb::TransformWindowPoint(cursorWindow, windowHandle, cursorPos);

		mouse_event(MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE, (DWORD)((pos.x / 2560.) * 65535.), (DWORD)((pos.y / 1440.) * 65535.), 0, 0);
		mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
		mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
		std::this_thread::sleep_for(delay);
	}

	// Move mouse back to original position
	mouse_event(MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE, (DWORD)((cursorPos.x / 2560.) * 65535.), (DWORD)((cursorPos.y / 1440.) * 65535.), 0, 0);
}