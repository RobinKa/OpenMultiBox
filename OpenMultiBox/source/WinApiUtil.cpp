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
	std::string WindowClassName;
	DWORD ProcessId;
};

static BOOL CALLBACK FindProcessWindowHandleCallback(HWND hwnd, LPARAM lParam)
{
	EnumWindowCallbackData* data = (EnumWindowCallbackData*)lParam;

	DWORD windowProcessId;
	GetWindowThreadProcessId(hwnd, &windowProcessId);

	char title[256];
	char className[256];

	if (windowProcessId == data->ProcessId &&
		GetWindowText(hwnd, title, 256) && title == data->WindowTitle &&
		GetClassName(hwnd, className, 256) && className == data->WindowClassName)
	{
		data->WindowHandles.push_back(hwnd);
	}

	return TRUE;
}

HWND omb::FindProcessWindowHandles(DWORD processId, const std::string& windowTitle, const std::string& windowClassName)
{
	EnumWindowCallbackData callbackData;
	callbackData.WindowTitle = windowTitle;
	callbackData.WindowClassName = windowClassName;
	callbackData.ProcessId = processId;
	callbackData.WindowHandles = std::vector<HWND>();

	EnumWindows((WNDENUMPROC)FindProcessWindowHandleCallback, (LPARAM)&callbackData);

	if (callbackData.WindowHandles.size() < 1)
	{
		throw std::exception("Failed to find process window");
	}
	else if (callbackData.WindowHandles.size() > 1)
	{
		throw std::exception("Found more than one process window");
	}

	return callbackData.WindowHandles[0];
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
	if (!GetWindowRect(originalWindowHandle, &originalRect))
	{
		throw std::exception("Failed to get original window rect");
	}

	RECT targetRect;
	if (!GetWindowRect(targetWindowHandle, &targetRect))
	{
		throw std::exception("Failed to get target window rect");
	}

	double origWidth = (double)originalRect.right - (double)originalRect.left;
	double origHeight = (double)originalRect.bottom - (double)originalRect.top;
	double targetWidth = (double)targetRect.right - (double)targetRect.left;
	double targetHeight = (double)targetRect.bottom - (double)targetRect.top;

	double relX = (point.x - (double)originalRect.left) / origWidth;
	double relY = (point.y - (double)originalRect.top) / origHeight;

	POINT transformed;
	transformed.x = (DWORD)(targetRect.left + relX * targetWidth);
	transformed.y = (DWORD)(targetRect.top + relY * targetHeight);

	return transformed;
}

void omb::ClickWindows(const std::vector<Window*>& windows, Window* lastWindow, int delayMs, bool left)
{
	const auto down = left ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_RIGHTDOWN;
	const auto up = left ? MOUSEEVENTF_LEFTUP : MOUSEEVENTF_RIGHTUP;

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
		auto windowHandle = window->GetHandle();

		const auto& pos = omb::TransformWindowPoint(cursorWindow, windowHandle, cursorPos);

		mouse_event(MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE, (DWORD)((pos.x / 2560.) * 65535.), (DWORD)((pos.y / 1440.) * 65535.), 0, 0);
		mouse_event(down, 0, 0, 0, 0);
		mouse_event(up, 0, 0, 0, 0);
		std::this_thread::sleep_for(delay);
	}

	if (primaryWindow)
	{
		auto windowHandle = primaryWindow->GetHandle();

		const auto& pos = omb::TransformWindowPoint(cursorWindow, windowHandle, cursorPos);

		mouse_event(MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE, (DWORD)((pos.x / 2560.) * 65535.), (DWORD)((pos.y / 1440.) * 65535.), 0, 0);
		mouse_event(down, 0, 0, 0, 0);
		mouse_event(up, 0, 0, 0, 0);
		std::this_thread::sleep_for(delay);
	}

	// Move mouse back to original position
	mouse_event(MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE, (DWORD)((cursorPos.x / 2560.) * 65535.), (DWORD)((cursorPos.y / 1440.) * 65535.), 0, 0);
}