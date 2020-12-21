#pragma execution_character_set("utf-8")

#include "WallpaperEngineClient.h"

#include <fstream>
#include <QWebEngineProfile>
#include <QSettings>
#include <QFileDialog>
#include <QMenu>
#include <QDesktopWidget>
#include <QApplication>
#include <QClipboard>
#include <QWebEngineSettings>
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
			loadUrl({ QFileDialog::getOpenFileName(this) });
		});

	mMenu->addAction("粘贴地址", [&]
		{
			loadUrl({ QApplication::clipboard()->text() });
		});

	mMenu->addAction("刷新壁纸", &desktopWebEngineView, &QWebEngineView::reload);

	auto autoRunAction = new QAction("开机启动", this);
	mMenu->addAction(autoRunAction);
	autoRunAction->setCheckable(true);
	autoRunAction->setChecked(autoRun());
	connect(autoRunAction, &QAction::toggled, &WallpaperEngineClient::setAutoRun);

	mMenu->addAction("退出程序", [=]
		{
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
	QWebEngineSettings::globalSettings()->setAttribute(QWebEngineSettings::PluginsEnabled, true);

	QFile configFile{ "wallpaper.json" };
	if (configFile.open(QFile::ReadOnly))
	{
		if (!loadUrl({ nlohmann::json::parse(configFile.readAll().constData())["url"].get<std::string>().c_str() }))
			loadUrl({ "https://github.com/thatboy-echo" });

		configFile.close();
	}
	else
		loadUrl({ "https://github.com/thatboy-echo" });
}

bool WallpaperEngineClient::loadUrl(QUrl url)
{
	if (url.isValid())
	{
		desktopWebEngineView.load(url);

		QFile configFile{ "wallpaper.json" };
		if (configFile.open(QFile::WriteOnly))
		{
			nlohmann::json j;
			j["url"] = desktopWebEngineView.url().toString().toStdString();
			configFile.write(QByteArray::fromStdString(j.dump()));
			configFile.close();
		}
	}

	return url.isValid();
}

void WallpaperEngineClient::setAutoRun(bool isAutoRun)
{
	const QString application_name = QApplication::applicationName();
	QSettings* settings = new QSettings(R"(HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Run)", QSettings::NativeFormat);
	if (isAutoRun)
		settings->setValue(application_name, QApplication::applicationFilePath().replace("/", "\\"));
	else
		settings->remove(application_name);
}

bool WallpaperEngineClient::autoRun()
{
	const QString application_name = QApplication::applicationName();
	QSettings* settings = new QSettings(R"(HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Run)", QSettings::NativeFormat);
	return settings->contains(application_name) && settings->value(application_name) == QApplication::applicationFilePath().replace("/", "\\");
}
