#pragma once

#include <Windows.h>

#include <string>
#include <utility>

namespace omb
{
	PROCESS_INFORMATION Launch(const std::string& path);
	HWND FindProcessWindowHandle(DWORD processId, const std::string& windowTitle);
	std::pair<int, int> GetMainScreenSize();
	bool IsFocusedWindow(HWND hwnd);
	POINT TransformWindowPoint(HWND originalWindowHandle, HWND targetWindowHandle, POINT point);
}