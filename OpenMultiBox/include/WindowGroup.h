#pragma once

#include "Window.h"

#include <vector>
#include <memory>
#include <functional>

namespace omb
{
	class WindowGroup
	{
	public:
		void AddWindow(Window* window);

		void SetupKeyboardBroadcastHook();

		void Rearrange();
		void RearrangeIfPrimaryChanged();
	private:
		std::vector<Window*> windows;
		Window* primaryWindow = nullptr;

		Window* GetFocusedWindow() const;

		HHOOK keyboardHookHandle;
	};
}