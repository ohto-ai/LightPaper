#pragma execution_character_set("utf-8")

#include "WallpaperEngineClient.h"
#include "SingleApplication.h"
#include <QMessageBox>

int main(int argc, char *argv[])
{
    SingleApplication a(argc, argv);

	if (!a.isRunning())
	{
		WallpaperEngineClient w;
		return a.exec();
	}
	return 0;
}
