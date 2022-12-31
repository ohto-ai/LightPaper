#pragma execution_character_set("utf-8")

#include "LightPaperClient.h"
#include "SingleApplication.h"
#include <QMessageBox>

int main(int argc, char* argv[])
{
	SingleApplication a(argc, argv);
	a.setApplicationName("LightPaper");
	a.setDesktopFileName("LightPaper");
	a.setApplicationVersion("v1.1.0");
	a.setOrganizationName("ohtoai");
	a.setOrganizationDomain("ohtoai.top");
	a.setWindowIcon(QIcon{ ":/LightPaperClient/icons/icons8-wallpaper-engine-96.png" });

#ifndef _DEBUG
	qputenv("QTWEBENGINEPROCESS_PATH", "WallpaperEngineInstance.exe");
#endif
	if (!a.instanceRunning())
	{
		LightPaperClient w;
		return a.exec();
	}
	return 0;
}