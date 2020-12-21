#pragma once

#include <QSystemTrayIcon>
#include <QWebEngineView>

class QSystemTrayIcon;

class WallpaperEngineClient
	: public QWidget
{
	Q_OBJECT

public:
	WallpaperEngineClient(QWidget* parent = Q_NULLPTR);
public:
	QSystemTrayIcon sysTrayIcon{ this };
	QWebEngineView desktopWebEngineView{ nullptr };
};
