#pragma once

#include <QApplication>
#include <QFile>
#include <QSettings>
#include <QSystemTrayIcon>
#include "WallpaperEngineView.h"

class QSystemTrayIcon;

class WallpaperEngineClient
	: public QWidget
{
	Q_OBJECT
public:
	void setUpSysTrayIcon();
	void setUpUi();
	void setUpLoadSignals();
	void setUpBindSignals();
	void initWallpaperUrl();
	void closeEvent(QCloseEvent* event) override;
	WallpaperEngineClient(QWidget* parent = Q_NULLPTR);
private:
	QSystemTrayIcon sysTrayIcon{ this };
	WallpaperEngineView desktopWebEngineView{ nullptr };

	static void setAutoRun(bool);
	static bool autoRun();
	static QUrl defaultWallPaperUrl();
	inline static QFile settingFile{ "wallpaper.json" };
	inline static QSettings* bootSetting{ new QSettings{
		R"(HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Run)"
		, QSettings::NativeFormat, qApp} };
};
