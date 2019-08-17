#include "Window.h"

#include <exception>

#include "WinApiUtil.h"

omb::Window::Window(const std::vector<HWND>& windowHandles)
{
	this->windowHandles = windowHandles;
}

void omb::Window::SetRect(int x, int y, int width, int height, bool topMost) const
{
	for (auto windowHandle : windowHandles)
	{
		if (!SetWindowPos(windowHandle, topMost ? HWND_TOPMOST : HWND_NOTOPMOST, x + 1, y + 1, width + 1, height + 1, SWP_NOOWNERZORDER))
		{
			throw std::exception("Failed to set window position");
		}
	}
}

std::vector<HWND> omb::Window::GetHandles() const
{
	return windowHandles;
}

bool omb::Window::IsFocused() const
{
	for (auto windowHandle : windowHandles)
	{
		if (omb::IsFocusedWindow(windowHandle))
		{
			return true;
		}
	}

	return false;
}

std::pair<int, int> omb::Window::GetPosition() const
{
	RECT rect;
	GetWindowRect(GetHandles()[0], &rect);

	return std::make_pair(rect.left, rect.top);
}

std::pair<int, int> omb::Window::GetSize() const
{
	RECT rect;
	GetWindowRect(GetHandles()[0], &rect);

	return std::make_pair(rect.right - rect.left, rect.bottom - rect.top);
}