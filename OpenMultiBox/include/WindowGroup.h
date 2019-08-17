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


		void LeftClick(int delayMs);

		void SetStayOnTop(bool b);
		bool GetStayOnTop() const;
		void SetBroadcastMovement(bool b);
		bool GetBroadcastMovement() const;
		void SetBroadcast(bool b);
		bool GetBroadcast() const;

		void SetWindowSwitching(bool b);
		bool GetWindowSwitching() const;

		void AddHotkeyCallback(DWORD key, std::function<void()> callback);
		const std::map<DWORD, std::function<void()>>& GetHotkeyCallbacks();

		void AddRearrangeCallback(std::function<void()> callback);

		const Window* GetPrimaryWindow() const;
		const std::vector<Window*>& GetWindows() const;
	private:
		std::map<DWORD, std::function<void()>> hotkeyCallbacks;
		std::vector<std::function<void()>> rearrangeCallbacks;

		std::vector<Window*> windows;

		Window* primaryWindow = nullptr;

		Window* GetFocusedWindow() const;

		HHOOK keyboardHookHandle;
		HHOOK mouseHookHandle;

		bool broadcast = false;
		bool stayOnTop = false;
		bool broadcastMovement = false;
		bool windowSwitching = true;
		bool clicking = false;
	};
}