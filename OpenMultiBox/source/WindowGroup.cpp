#include "WindowGroup.h"

#include "WinApiUtil.h"

#include <iostream>

void omb::WindowGroup::AddWindow(Window* window)
{
	windows.push_back(window);

	if (!primaryWindow)
	{
		primaryWindow = window;
	}
}

void omb::WindowGroup::Rearrange()
{
	auto screenSize = omb::GetMainScreenSize();
	auto windowCount = windows.size();

	std::pair<int, int> secondarySize = std::make_pair((int)(screenSize.first / 5.0), (int)(screenSize.second / 5.0));

	int primaryWidth = screenSize.first - secondarySize.first;
	int primaryHeight = (int)(screenSize.second * ((double)primaryWidth / screenSize.first));

	int secondaryIndex = 0;

	primaryWindow = GetFocusedWindow();

	for (auto window : windows)
	{
		if (window == primaryWindow)
		{
			window->SetRect(0, 0, primaryWidth, primaryHeight);
		}
		else
		{
			window->SetRect(primaryWidth, secondaryIndex * secondarySize.second, secondarySize.first, secondarySize.second);
			secondaryIndex++;
		}
	}
}

void omb::WindowGroup::RearrangeIfPrimaryChanged()
{
	if (GetFocusedWindow() != primaryWindow)
	{
		Rearrange();
	}
}

omb::Window* omb::WindowGroup::GetFocusedWindow() const
{
	for (auto window : windows)
	{
		if (omb::IsFocusedWindow(window->GetHandle()))
		{
			return window;
		}
	}

	return nullptr;
}