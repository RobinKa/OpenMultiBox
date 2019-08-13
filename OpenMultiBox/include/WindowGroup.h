#pragma once

#include "Window.h"

#include <vector>
#include <memory>

namespace omb
{
	class WindowGroup
	{
	public:
		void AddWindow(Window* window);

		void Rearrange();
		void RearrangeIfPrimaryChanged();
	private:
		std::vector<Window*> windows;
		Window* primaryWindow = nullptr;

		Window* GetFocusedWindow() const;
	};
}