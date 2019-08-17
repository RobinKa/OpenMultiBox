#pragma once

#include <thread>
#include <map>

class QWidget;
class QApplication;

namespace omb
{
	class UserInterface
	{
	public:
		void Start();
		void Stop();
		int CreateIdWindow();
		void SetIdWindowPosition(int windowId, int x, int y);

	private:
		std::thread uiThread;
		QApplication* app = nullptr;

		std::map<int, QWidget*> idWindows;
		int nextWindowId = 0;
	};
}