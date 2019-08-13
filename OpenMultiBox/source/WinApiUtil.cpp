#include "WinApiUtil.h"

#include <vector>

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

HWND omb::FindProcessWindowHandle(DWORD processId, const std::string& windowTitle)
{
	EnumWindowCallbackData callbackData;
	callbackData.WindowTitle = windowTitle;
	callbackData.ProcessId = processId;

	EnumWindows((WNDENUMPROC)FindProcessWindowHandleCallback, (LPARAM)&callbackData);

	if (callbackData.WindowHandles.size() < 2)
	{
		throw std::exception("Failed to find process window");
	}

	return callbackData.WindowHandles[1];
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