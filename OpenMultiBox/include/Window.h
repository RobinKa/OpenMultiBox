#pragma once

#include <Windows.h>

namespace omb
{
	class Window
	{
	public:
		Window(HWND windowHandle);

		void SetRect(int x, int y, int width, int height, HWND insertAfter) const;

		HWND GetHandle() const;
	private:
		HWND windowHandle;
	};
}