﻿#pragma execution_character_set("utf-8")

#include "WallpaperEngineClient.h"

#include <QWebEngineProfile>
#include <QFileDialog>
#include <QMenu>
#include <QApplication>
#include <QClipboard>
#include <QCloseEvent>
#include <json.hpp>

WallpaperEngineClient::WallpaperEngineClient(QWidget* parent)
	: QWidget(parent)
{
	setUpUi();
	setUpSysTrayIcon();
	setUpBindSignals();
	setUpLoadSignals();
	initWallpaperUrl();
}

void WallpaperEngineClient::setUpSysTrayIcon()
{
	sysTrayIcon.setIcon(QIcon(":WallpaperEngineClient/icons/icons8-wallpaper-engine-96.png"));
	sysTrayIcon.setToolTip("Wallpaper Engine Client");

	auto mMenu = new QMenu{ this };
	mMenu->addAction("浏览文件", [&]
		{
			const auto path = QFileDialog::getOpenFileName(this, ""
				, desktopWebEngineView.url().isLocalFile() ? desktopWebEngineView.url().toLocalFile() : ""
				, "图像文件(*.jpg;*.jpeg;*.png;*.bmp;*.gif;*.webp;*.ico);;网页文件(*.htm;*.html);;所有文件(*.*)");
			if (!path.isEmpty())
				desktopWebEngineView.load(QUrl::fromLocalFile(path));
		});

	mMenu->addAction("粘贴地址", [&]
		{
			const auto clipboardUrlText{ QApplication::clipboard()->text() };
			if (!QRegExp{ R"([a-zA-Z]:.*)" }.exactMatch(clipboardUrlText))
				desktopWebEngineView.load(clipboardUrlText);
		});

	mMenu->addAction("刷新壁纸", &desktopWebEngineView, &QWebEngineView::reload);

	auto autoRunAction = new QAction("开机启动", this);
	mMenu->QWidget::addAction(autoRunAction);
	autoRunAction->setCheckable(true);
	autoRunAction->setChecked(autoRun());
	connect(autoRunAction, &QAction::toggled, &WallpaperEngineClient::setAutoRun);

	mMenu->addAction("退出程序", this, &QWidget::close);

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

void WallpaperEngineClient::setUpLoadSignals()
{
	connect(&desktopWebEngineView, &QWebEngineView::loadStarted, [&]
		{
			sysTrayIcon.showMessage("正在加载壁纸", "坐和放宽！", QSystemTrayIcon::MessageIcon::Information);
		});
	connect(&desktopWebEngineView, &QWebEngineView::loadFinished, [=](bool success)
		{
			if (!success)
				sysTrayIcon.showMessage("壁纸加载失败", "请检查此链接或文件的可用性！", QSystemTrayIcon::MessageIcon::Warning);
			else
				sysTrayIcon.showMessage("壁纸加载成功", "开始你的表演！", QSystemTrayIcon::MessageIcon::Information);
		});
	connect(&desktopWebEngineView, &QWebEngineView::loadFinished, [=](bool success)
		{
			if (success && settingFile.open(QFile::WriteOnly))
			{
				nlohmann::json j;
				j["url"] = desktopWebEngineView.url().toString().toStdString();
				settingFile.write(QByteArray::fromStdString(j.dump()));
				settingFile.close();
			}
		});
}

void WallpaperEngineClient::setUpBindSignals()
{
	connect(&desktopWebEngineView, &WallpaperEngineView::AttachedWorkerW, [=](bool success) {
		if (success)
			sysTrayIcon.showMessage("桌面绑定成功", "快来舔屏吧!", QSystemTrayIcon::MessageIcon::Information);
		else
			sysTrayIcon.showMessage("屏幕绑定失败", "你不爱我了，居然装着别的壁纸软件!", QSystemTrayIcon::MessageIcon::Critical);
		});
}

void WallpaperEngineClient::initWallpaperUrl()
{
	if (settingFile.open(QFile::ReadOnly))
	{
		const QUrl url{ nlohmann::json::parse(settingFile.readAll().constData())["url"].get<std::string>().c_str() };
		settingFile.close();
		if (url.isValid())
			desktopWebEngineView.load(url);
		else
			desktopWebEngineView.load(defaultWallPaperUrl());
	}
	else
		desktopWebEngineView.load(defaultWallPaperUrl());
}

void WallpaperEngineClient::closeEvent(QCloseEvent* event)
{
	desktopWebEngineView.close();
	event->accept();
}