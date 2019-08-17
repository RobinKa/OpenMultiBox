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

			mainWindow = new QMainWindow();
			mainWindow->resize(1024, 200);
			mainWindow->setWindowFlags(Qt::Widget | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::WindowTransparentForInput);
			mainWindow->setParent(0);
			mainWindow->setAttribute(Qt::WA_NoSystemBackground, true);
			mainWindow->setAttribute(Qt::WA_TranslucentBackground, true);
			mainWindow->show();
			
			broadcastLabel = new QLabel("", mainWindow);
			broadcastLabel->resize(512, 200);
			QPalette palette = broadcastLabel->palette();
			palette.setColor(broadcastLabel->foregroundRole(), Qt::yellow);
			broadcastLabel->setPalette(palette);
			broadcastLabel->move(0, 80);
			broadcastLabel->setFont(QFont("Arial", 14, QFont::Bold));
			SetBroadcast(false);
			broadcastLabel->show();

			movementBroadcastLabel = new QLabel("", mainWindow);
			movementBroadcastLabel->resize(512, 200);
			palette = movementBroadcastLabel->palette();
			palette.setColor(movementBroadcastLabel->foregroundRole(), Qt::yellow);
			movementBroadcastLabel->setPalette(palette);
			movementBroadcastLabel->move(0, 20);
			movementBroadcastLabel->setFont(QFont("Arial", 14, QFont::Bold));
			SetMovementBroadcast(false);
			movementBroadcastLabel->show();

			stayOnTopLabel = new QLabel("", mainWindow);
			stayOnTopLabel->resize(512, 200);
			palette = stayOnTopLabel->palette();
			palette.setColor(stayOnTopLabel->foregroundRole(), Qt::yellow);
			stayOnTopLabel->setPalette(palette);
			stayOnTopLabel->move(0, 40);
			stayOnTopLabel->setFont(QFont("Arial", 14, QFont::Bold));
			SetStayOnTop(false);
			stayOnTopLabel->show();

			mouseBroadcastLabel = new QLabel("[OEM1] Mouse broadcast", mainWindow);
			mouseBroadcastLabel->resize(512, 200);
			palette = mouseBroadcastLabel->palette();
			palette.setColor(mouseBroadcastLabel->foregroundRole(), Qt::yellow);
			mouseBroadcastLabel->setPalette(palette);
			mouseBroadcastLabel->move(0, 0);
			mouseBroadcastLabel->setFont(QFont("Arial", 14, QFont::Bold));
			mouseBroadcastLabel->show();

			copyWTFLabel = new QLabel("[F9] Copy WTF", mainWindow);
			copyWTFLabel->resize(512, 200);
			palette = copyWTFLabel->palette();
			palette.setColor(copyWTFLabel->foregroundRole(), Qt::yellow);
			copyWTFLabel->setPalette(palette);
			copyWTFLabel->move(0, 60);
			copyWTFLabel->setFont(QFont("Arial", 14, QFont::Bold));
			copyWTFLabel->show();

			return app->exec();
		});
	}
}

void omb::UserInterface::Stop()
{
	if (app)
	{
		QMetaObject::invokeMethod(app, [this]()
		{
			app->quit();
		});

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

void omb::UserInterface::SetMovementBroadcast(bool b)
{
	QMetaObject::invokeMethod(app, [this, b]()
	{
		movementBroadcastLabel->setText(QString("[F7] Movement broadcast: %1").arg(b ? "Yes" : "No"));
	});
}

void omb::UserInterface::SetStayOnTop(bool b)
{
	QMetaObject::invokeMethod(app, [this, b]()
	{
		stayOnTopLabel->setText(QString("[F8] Stay on top: %1").arg(b ? "Yes" : "No"));
	});
}

void omb::UserInterface::SetBroadcast(bool b)
{
	QMetaObject::invokeMethod(app, [this, b]()
	{
		broadcastLabel->setText(QString("[F10 / F11] Broadcast: %1").arg(b ? "Yes" : "No"));
	});
}

void omb::UserInterface::SetMainWindowPosition(int x, int y)
{
	QMetaObject::invokeMethod(app, [this, x, y]()
	{
		mainWindow->move(x - 120, y - 80);
	});
}