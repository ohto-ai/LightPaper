#include "WallpaperEngineView.h"
#include <Windows.h>
#include <QWebEngineSettings>

namespace workerW
{
	extern "C"
	{
		HWND handleProgramManagerWindow();
		void splitOutWorkerW();
		void destroyWorkerW();
		HWND getWorkerW();
		BOOL CALLBACK enumWorkWProc(HWND, LPARAM);
	}
}

WallpaperEngineView::WallpaperEngineView(QWidget* parent)
	: QWebEngineView(parent)
{
	settings()->setAttribute(QWebEngineSettings::PluginsEnabled, true);
	workerW::splitOutWorkerW();
	emit workerWAttached(attachWorkW());
}

bool WallpaperEngineView::attachWorkW()
{
	const auto handleWorkerW = workerW::getWorkerW();
	if (!handleWorkerW)
		return false;
	SetParent(reinterpret_cast<HWND>(winId()), handleWorkerW);
	setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnBottomHint);
	setWindowState(Qt::WindowNoState);
	setFocusPolicy(Qt::NoFocus);
	showFullScreen();
	return true;
}

WallpaperEngineView::~WallpaperEngineView()
{
	workerW::destroyWorkerW();
}

extern "C"
{
	HWND workerW::handleProgramManagerWindow()
	{
		static HWND handleProgramManagerWindow{ nullptr };
		if (!handleProgramManagerWindow)
			handleProgramManagerWindow = ::FindWindow(TEXT("Progman"), TEXT("Program Manager"));
		return handleProgramManagerWindow;
	}

	void workerW::splitOutWorkerW()
	{
		DWORD_PTR result;
		::SendMessageTimeout(handleProgramManagerWindow(), 0x052c, 0, 0
			, SMTO_NORMAL, 1000, &result);
	}

	void workerW::destroyWorkerW()
	{
		DWORD_PTR result;
		::SendMessageTimeout(handleProgramManagerWindow(), 0x052c, 1, 0
			, SMTO_NORMAL, 1000, &result);
	}

	BOOL CALLBACK workerW::enumWorkWProc(HWND handleTopWindow, LPARAM lpHandleWorkerW)
	{
		if (FindWindowEx(handleTopWindow, nullptr, TEXT("SHELLDLL_DefView"), nullptr) == nullptr)
			return TRUE;
		*reinterpret_cast<HWND*>(lpHandleWorkerW) = FindWindowEx(nullptr, handleTopWindow, TEXT("WorkerW"), nullptr);
		return FALSE;
	}

	HWND workerW::getWorkerW() {
		HWND handleWorkerW{ nullptr };
		EnumWindows(&enumWorkWProc, reinterpret_cast<LPARAM>(&handleWorkerW));
		return handleWorkerW;
	}
}