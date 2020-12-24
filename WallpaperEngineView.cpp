#include "WallpaperEngineView.h"
#include <Windows.h>
#include <QWebEngineSettings>

namespace workerW
{
	void splitOutWorkerW();
	void destroyWorkerW();
	HWND getWorkerW();
}

WallpaperEngineView::WallpaperEngineView(QWidget *parent)
	: QWebEngineView(parent)
{
	settings()->setAttribute(QWebEngineSettings::PluginsEnabled, true);

	workerW::splitOutWorkerW();
	if (!bindToWorkW())
	{
		workerW::destroyWorkerW();
		workerW::splitOutWorkerW();
		if (bindToWorkW())
			emit workerWBinded(false);
	}
	emit workerWBinded(true);	
}

bool WallpaperEngineView::bindToWorkW()
{
	const auto handleWorkerW = workerW::getWorkerW();
	if (!handleWorkerW)
		return false;
	SetParent(reinterpret_cast<HWND>(winId()), handleWorkerW);
	setWindowFlags(Qt::Window | Qt::FramelessWindowHint  | Qt::WindowStaysOnBottomHint);
	setWindowState(Qt::WindowNoState);
	setFocusPolicy(Qt::NoFocus);
	showFullScreen();
	return true;
}

WallpaperEngineView::~WallpaperEngineView()
{
	workerW::destroyWorkerW();
}

void workerW::splitOutWorkerW()
{
	const HWND handleProgmanWindow = ::FindWindow(TEXT("Progman"), TEXT("Program Manager"));
	if (!handleProgmanWindow)
		return;
	DWORD_PTR result;
	::SendMessageTimeout(handleProgmanWindow, 0x052c, 0, 0
		, SMTO_NORMAL, 1000, &result);
}

void workerW::destroyWorkerW()
{
	const HWND handleProgmanWindow = ::FindWindow(TEXT("Progman"), TEXT("Program Manager"));
	if (!handleProgmanWindow)
		return;
	DWORD_PTR result;
	::SendMessageTimeout(handleProgmanWindow, 0x052c, 1, 0
		, SMTO_NORMAL, 1000, &result);
}

HWND workerW::getWorkerW() {
	const HWND handleProgmanWindow = ::FindWindow(TEXT("Progman"), TEXT("Program Manager"));
	if (!handleProgmanWindow)
		return nullptr;
	HWND handleWorkerW = ::FindWindowEx(nullptr, nullptr, TEXT("WorkerW"), nullptr);
	while (handleWorkerW && ::FindWindowEx(nullptr, handleWorkerW, nullptr, nullptr) != handleProgmanWindow)
		handleWorkerW = ::FindWindowEx(nullptr, handleWorkerW, TEXT("WorkerW"), nullptr);
	return handleWorkerW;
}