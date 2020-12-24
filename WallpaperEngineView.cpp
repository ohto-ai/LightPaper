#include "WallpaperEngineView.h"
#include <Windows.h>
#include <QWebEngineSettings>

namespace workerW
{
	void splitOutWorkerW();
	void destroyWorkerW();
	HWND getWorkerW();
	inline BOOL CALLBACK EnumWorkWProc(_In_ HWND, _In_ LPARAM);
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

inline BOOL CALLBACK workerW::EnumWorkWProc(_In_ HWND handleTopWindow, _In_ LPARAM lpHandleWorkerW)
{
	if (FindWindowEx(handleTopWindow, nullptr, TEXT("SHELLDLL_DefView"), nullptr) != nullptr)
	{
		*reinterpret_cast<HWND*>(lpHandleWorkerW) = FindWindowEx(nullptr, handleTopWindow, TEXT("WorkerW"), nullptr);
		return FALSE;
	}
	return TRUE;
}

HWND workerW::getWorkerW() {
	const HWND handleProgramManagerWindow = ::FindWindow(TEXT("Progman"), TEXT("Program Manager"));
	if (!handleProgramManagerWindow)
		return nullptr;
	HWND handleWorkerW{ nullptr };
	EnumWindows(EnumWorkWProc, reinterpret_cast<LPARAM>(&handleWorkerW));
	return handleWorkerW;
}