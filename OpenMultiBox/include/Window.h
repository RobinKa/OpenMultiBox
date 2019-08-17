#pragma once

#include <Windows.h>

#include <vector>

namespace omb
{
	class Window
	{
	public:
		Window(HWND windowHandle);

		void SetRect(int x, int y, int width, int height, bool topMost) const;

		HWND GetHandle() const;
		std::pair<int, int> GetPosition() const;
		std::pair<int, int> GetSize() const;

		bool IsFocused() const;
	private:
		HWND windowHandle;
	};
}