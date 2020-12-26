#pragma once

#include <QWebEngineView>

class WallpaperEngineView : public QWebEngineView
{
	Q_OBJECT

public:
	WallpaperEngineView(QWidget* parent);
	virtual ~WallpaperEngineView();
	bool bindToWorkW();
signals:
	void AttachedWorkerW(bool);
};
