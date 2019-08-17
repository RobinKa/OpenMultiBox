#include "UserInterface.h"

#include <QtWidgets>

void omb::UserInterface::Start()
{
	if (!app)
	{
		uiThread = std::thread([this]()
		{
			int argc = 0;
			char** argv = nullptr;

			app = new QApplication(argc, argv);
			return app->exec();
		});
	}
}

void omb::UserInterface::Stop()
{
	if (app)
	{
		app->quit();
		uiThread.join();
		delete app;
		app = nullptr;
	}
}

int omb::UserInterface::CreateIdWindow()
{
	int windowId = -1;

	if (app)
	{
		windowId = nextWindowId++;

		QMetaObject::invokeMethod(app, [this, windowId]()
		{
			QWidget* window = new QWidget();
			window->resize(64, 64);
			window->setWindowFlags(Qt::Widget | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::WindowTransparentForInput);
			window->setParent(0);
			window->setAttribute(Qt::WA_NoSystemBackground, true);
			window->setAttribute(Qt::WA_TranslucentBackground, true);

			window->show();

			QLabel* idLabel = new QLabel(QString::number(windowId), window);
			idLabel->setFont(QFont("Arial", 32, QFont::Bold));

			QPalette palette = idLabel->palette();
			palette.setColor(idLabel->backgroundRole(), Qt::yellow);
			palette.setColor(idLabel->foregroundRole(), Qt::yellow);
			idLabel->setPalette(palette);

			idLabel->show();

			idWindows.emplace(windowId, window);
		});
	}

	return windowId;
}

void omb::UserInterface::SetIdWindowPosition(int windowId, int x, int y)
{
	QMetaObject::invokeMethod(app, [this, windowId, x, y]()
	{
		idWindows[windowId]->move(x + 10, y + 10);
	});
}