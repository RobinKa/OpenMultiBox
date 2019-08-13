#pragma once

#include "Window.h"

#include <vector>
#include <map>
#include <memory>
#include <functional>

namespace omb
{
	class WindowGroup
	{
	public:
		void AddWindow(Window* window);

		void SetupKeyboardBroadcastHook();
		void SetupMouseBroadcastHook();

		void RemoveHooks();

		void Rearrange();
		void RearrangeIfPrimaryChanged();

		void AddHotkeyCallback(DWORD key, std::function<void()> callback);

		void SetStayOnTop(bool b);
		bool GetStayOnTop() const;
	private:
		std::map<DWORD, std::function<void()>> hotkeyCallbacks;

		std::vector<Window*> windows;
		Window* primaryWindow = nullptr;

		Window* GetFocusedWindow() const;

		HHOOK keyboardHookHandle;
		HHOOK mouseHookHandle;

		bool broadcast = false;
		bool stayOnTop = false;
	};
}