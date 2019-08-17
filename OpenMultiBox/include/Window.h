#pragma once

#include <Windows.h>

#include <vector>

namespace omb
{
	class Window
	{
	public:
		Window(const std::vector<HWND>& windowHandle);

		void SetRect(int x, int y, int width, int height, bool topMost) const;

		std::vector<HWND> GetHandles() const;
		std::pair<int, int> GetPosition() const;

		bool IsFocused() const;
	private:
		std::vector<HWND> windowHandles;
	};
}