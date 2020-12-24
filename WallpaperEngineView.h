#pragma once

#include <QWebEngineView>

class WallpaperEngineView : public QWebEngineView
{
	Q_OBJECT

public:
	WallpaperEngineView(QWidget *parent);
	~WallpaperEngineView();

	bool bindToWorkW();
signals:
	void workerWBinded(bool);
};
