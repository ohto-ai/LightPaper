#pragma once

#include <QApplication>
#include <QFile>
#include <QSettings>
#include <QSystemTrayIcon>
#include <QWebEngineView>

class QSystemTrayIcon;

class WallpaperEngineClient
	: public QWidget
{
	Q_OBJECT

public:
	void setUpSysTrayIcon();
	void setUpUi();
	void bindToWorkW();
	WallpaperEngineClient(QWidget* parent = Q_NULLPTR);
public:
	QSystemTrayIcon sysTrayIcon{ this };
	QWebEngineView desktopWebEngineView{ nullptr };
	bool loadUrl(const QUrl& url);
	static void setAutoRun(bool);
	static bool autoRun();
	QFile settingFile{ "wallpaper.json" };
	static QUrl defaultWallPaperUrl();
	inline static QSettings* bootSetting{ new QSettings{
		R"(HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Run)"
		, QSettings::NativeFormat, qApp} };
};
