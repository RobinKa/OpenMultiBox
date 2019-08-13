#include "Window.h"

#include <exception>

omb::Window::Window(HWND windowHandle)
{
	this->windowHandle = windowHandle;
}

void omb::Window::SetRect(int x, int y, int width, int height, bool topMost) const
{
	if (!SetWindowPos(windowHandle, topMost ? HWND_TOPMOST : HWND_NOTOPMOST, x + 1, y + 1, width + 1, height + 1, SWP_NOOWNERZORDER))
	{
		throw std::exception("Failed to set window position");
	}
}

HWND omb::Window::GetHandle() const
{
	return windowHandle;
}