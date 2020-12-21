#pragma execution_character_set("utf-8")

#include "WallpaperEngineClient.h"

#include <fstream>
#include <QWebEngineProfile>
#include <QFile>
#include <QFileDialog>
#include <QMenu>
#include <QDesktopWidget>
#include <QApplication>
#include <QClipboard>
#include <QMessageBox>
#include <Windows.h>
#include <json.hpp>


static HWND handleWorkerW = nullptr;
inline BOOL CALLBACK enumWindowsProc(_In_ HWND tophandle, _In_ LPARAM topparamhandle)
{
	if (FindWindowEx(tophandle, nullptr
		, TEXT("SHELLDLL_DefView"), nullptr) != nullptr)
		handleWorkerW = FindWindowEx(nullptr, tophandle
			, TEXT("WorkerW"), nullptr);
	return TRUE;
}

HWND getWorkerW() {
	int result;
	HWND handleProgmanWindow = FindWindow(TEXT("Progman"), nullptr);
	SendMessageTimeout(handleProgmanWindow, 0x052c, 0, 0
		, SMTO_NORMAL, 0x3e8, reinterpret_cast<PDWORD_PTR>(&result));
	EnumWindows(enumWindowsProc, 0);
	ShowWindow(handleWorkerW, SW_HIDE);
	return handleProgmanWindow;
}

WallpaperEngineClient::WallpaperEngineClient(QWidget* parent)
	: QWidget(parent)
{
	setStyleSheet(R"(
*{
	font-family:  "Arial", "Segoe UI", "黑体", "等线";
	color: white;
	background:rgb(50, 50, 50);
})");

	qputenv("QTWEBENGINEPROCESS_PATH", "WallpaperEngineInstance.exe");

	sysTrayIcon.setIcon(QIcon(":WallpaperEngineClient/icons/icons8-wallpaper-engine-96.png"));
	sysTrayIcon.setToolTip("WallpaperEngineClient");

	auto mMenu = new QMenu{ this };
	mMenu->addAction("浏览文件", [&]
		{
			QUrl url{ QFileDialog::getOpenFileName(this) };
			if (url.isValid())
				desktopWebEngineView.load(url);
		});
	
	mMenu->addAction("粘贴地址", [&]
		{
			QUrl url{ QApplication::clipboard()->text() };
			if (url.isValid())
				desktopWebEngineView.load(url);
		});
	
	mMenu->addAction("刷新壁纸", &desktopWebEngineView, &QWebEngineView::reload);
		
	mMenu->addAction("退出程序", [=]
		{
			QFile configFile{ "wallpaper.json" };
			if (configFile.open(QFile::WriteOnly))
			{
				nlohmann::json j;
				j["url"] = desktopWebEngineView.url().toString().toStdString();
				configFile.write(QByteArray::fromStdString(j.dump()));
				configFile.close();
			}
			qApp->quit();
		});

	sysTrayIcon.setContextMenu(mMenu);
	sysTrayIcon.show();

	static HWND desktopWebEngineViewId;
	desktopWebEngineViewId = reinterpret_cast<HWND>(desktopWebEngineView.winId());
	SetParent(desktopWebEngineViewId, getWorkerW());
	SetWindowPos(desktopWebEngineViewId, HWND_TOP, 0, 0, 0, 0
		, WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_RIGHTSCROLLBAR | WS_EX_NOACTIVATE);

	desktopWebEngineView.move(QApplication::desktop()->pos());
	desktopWebEngineView.resize(QApplication::desktop()->size());
	desktopWebEngineView.showFullScreen();

	QFile configFile{ "wallpaper.json" };
	if(configFile.open(QFile::ReadOnly))
	{
		nlohmann::json j = nlohmann::json::parse(configFile.readAll().constData());
		QUrl url{ j["url"].get<std::string>().c_str() };

		QMessageBox::information(this, "", url.toString());
		if (url.isValid())
			desktopWebEngineView.load(url);
		else
			desktopWebEngineView.load(QUrl("https://github.com/thatboy-echo"));

		configFile.close();
	}
	else
		desktopWebEngineView.load(QUrl("https://github.com/thatboy-echo"));
}