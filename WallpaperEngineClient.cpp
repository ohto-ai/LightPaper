#pragma execution_character_set("utf-8")

#include "WallpaperEngineClient.h"

#include <QWebEngineProfile>
#include <QSettings>
#include <QFileDialog>
#include <QMenu>
#include <QTimer>
#include <QDesktopWidget>
#include <QApplication>
#include <QClipboard>
#include <QWebEngineSettings>
#include <Windows.h>
#include <json.hpp>

namespace workerW
{
	inline BOOL CALLBACK enumWorkWProc(_In_ HWND topHandle, _In_ LPARAM handleWorkerW)
	{
		if (FindWindowEx(topHandle, nullptr
			, TEXT("SHELLDLL_DefView"), nullptr) != nullptr)
			*reinterpret_cast<HWND*>(handleWorkerW) = FindWindowEx(nullptr, topHandle
				, TEXT("WorkerW"), nullptr);
		return TRUE;
	}
	HWND getWorkerW() {
		int result;
		const HWND handleWorkerW = FindWindow(TEXT("Progman"), nullptr);
		if (!handleWorkerW)
			return nullptr;
		SendMessageTimeout(handleWorkerW, 0x052c, 0, 0
			, SMTO_NORMAL, 0x3e8, reinterpret_cast<PDWORD_PTR>(&result));
		HWND handleWorkerWOld = nullptr;
		EnumWindows(enumWorkWProc, reinterpret_cast<LPARAM>(&handleWorkerWOld));
		if (handleWorkerWOld)
			ShowWindow(handleWorkerWOld, SW_HIDE);
		return handleWorkerW;
	}
}

void WallpaperEngineClient::setUpSysTrayIcon()
{
	sysTrayIcon.setIcon(QIcon(":WallpaperEngineClient/icons/icons8-wallpaper-engine-96.png"));
	sysTrayIcon.setToolTip("Wallpaper Engine Client");

	auto mMenu = new QMenu{ this };
	mMenu->addAction("浏览文件", [&]
		{
			loadUrl(QUrl::fromLocalFile(QFileDialog::getOpenFileName(this)));
		});

	mMenu->addAction("粘贴地址", [&]
		{
			const auto clipboardUrlText{ QApplication::clipboard()->text() };
			if (!QRegExp{ R"([a-zA-Z]:.*)" }.exactMatch(clipboardUrlText))
				loadUrl(clipboardUrlText);
		});

	mMenu->addAction("刷新壁纸", &desktopWebEngineView, &QWebEngineView::reload);
	
	mMenu->addAction("重新加载", this, &WallpaperEngineClient::bindToWorkW);

	auto autoRunAction = new QAction("开机启动", this);
	mMenu->QWidget::addAction(autoRunAction);
	autoRunAction->setCheckable(true);
	autoRunAction->setChecked(autoRun());
	connect(autoRunAction, &QAction::toggled, &WallpaperEngineClient::setAutoRun);

	mMenu->addAction("退出程序", [=]
		{
			qApp->quit();
		});

	sysTrayIcon.setContextMenu(mMenu);
	sysTrayIcon.show();
}

void WallpaperEngineClient::setUpUi()
{
	QFile qssFile{ ":/WallpaperEngineClient/.qss" };
	qssFile.open(QFile::ReadOnly);
	setStyleSheet(qssFile.readAll());
	qssFile.close();
}

void WallpaperEngineClient::bindToWorkW()
{
	const auto desktopWebEngineViewId{ reinterpret_cast<HWND>(desktopWebEngineView.winId()) };
	SetParent(desktopWebEngineViewId, workerW::getWorkerW());
	SetWindowPos(desktopWebEngineViewId, HWND_TOP, 0, 0, 0, 0
		, WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_RIGHTSCROLLBAR | WS_EX_NOACTIVATE);
	desktopWebEngineView.showFullScreen();
}

WallpaperEngineClient::WallpaperEngineClient(QWidget* parent)
	: QWidget(parent)
{
	setUpUi();
	setUpSysTrayIcon();

	if (QApplication::arguments().contains("/onboot"))
		QTimer::singleShot(5000, this, &WallpaperEngineClient::bindToWorkW);
	else
		bindToWorkW();

	QWebEngineSettings::globalSettings()->setAttribute(QWebEngineSettings::PluginsEnabled, true);

	connect(QApplication::desktop(), &QDesktopWidget::resized, this, &WallpaperEngineClient::bindToWorkW);

	if (settingFile.open(QFile::ReadOnly))
	{
		const QUrl url{ nlohmann::json::parse(settingFile.readAll().constData())["url"].get<std::string>().c_str() };
		settingFile.close();
		if (!loadUrl(url))
			loadUrl(defaultWallPaperUrl());
	}
	else
		loadUrl(defaultWallPaperUrl());
}

bool WallpaperEngineClient::loadUrl(const QUrl& url)
{
	if (url.isValid())
	{
		if (url.isLocalFile() && !QFile::exists(url.toLocalFile()))
			return false;
		desktopWebEngineView.load(url);

		if (settingFile.open(QFile::WriteOnly))
		{
			nlohmann::json j;
			j["url"] = url.toString().toStdString();
			settingFile.write(QByteArray::fromStdString(j.dump()));
			settingFile.close();
		}
	}

	return url.isValid();
}

void WallpaperEngineClient::setAutoRun(bool isAutoRun)
{
	const QString application_name = QApplication::applicationName();
	if (isAutoRun)
		bootSetting->setValue(application_name, QString(R"(%1 %2)").arg(QApplication::applicationFilePath().replace("/", "\\"), "/onboot"));
	else
		bootSetting->remove(application_name);
}

bool WallpaperEngineClient::autoRun()
{
	return bootSetting->contains(QApplication::applicationName());
}

QUrl WallpaperEngineClient::defaultWallPaperUrl()
{
	QFile takanshiRikkaFile{ QApplication::applicationDirPath() + "/content/Takanashi_Rikka.gif" };
	if (!takanshiRikkaFile.exists())
	{
		QFile takanshiRikkaRC{ ":/WallpaperEngineClient/content/Takanashi_Rikka.gif" };
		takanshiRikkaRC.open(QFile::ReadOnly);

		const QDir takanshiRikkaDir{ QApplication::applicationDirPath() };
		takanshiRikkaDir.mkpath({ QFileInfo{ takanshiRikkaFile.fileName() }.path() });

		if (takanshiRikkaFile.open(QFile::WriteOnly))
		{
			takanshiRikkaFile.write(takanshiRikkaRC.readAll());
			takanshiRikkaFile.close();
		}
		takanshiRikkaRC.close();
	}
	return QUrl::fromLocalFile(takanshiRikkaFile.fileName());
}
