#include "Window.h"

#include <exception>

#include "WinApiUtil.h"

omb::Window::Window(HWND windowHandle)
{
	this->windowHandle = windowHandle;
}

void omb::Window::SetRect(int x, int y, int width, int height, bool topMost) const
{
	if (!SetWindowPos(windowHandle, topMost ? HWND_TOPMOST : HWND_NOTOPMOST, x, y, width, height, SWP_NOOWNERZORDER))
	{
		throw std::exception("Failed to set window position");
	}
}

HWND omb::Window::GetHandle() const
{
	return windowHandle;
}

bool omb::Window::IsFocused() const
{
	return omb::IsFocusedWindow(windowHandle);
}

std::pair<int, int> omb::Window::GetPosition() const
{
	RECT rect;
	GetWindowRect(GetHandle(), &rect);

	return std::make_pair(rect.left, rect.top);
}

std::pair<int, int> omb::Window::GetSize() const
{
	RECT rect;
	GetWindowRect(GetHandle(), &rect);

	return std::make_pair(rect.right - rect.left, rect.bottom - rect.top);
}