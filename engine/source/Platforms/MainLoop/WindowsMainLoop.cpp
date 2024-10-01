#include "pch.h"
#include "WindowsMainAppLoop.h"

using namespace Zzz::Platforms;

#ifdef _WINDOWS

WindowsMainAppLoop::WindowsMainAppLoop(unique_ptr<IWinApp> _win, unique_ptr<IGAPI> _gapi) :
	IMainAppLoop(move(_win), move(_gapi))
{
}

WindowsMainAppLoop::~WindowsMainAppLoop()
{
}

void WindowsMainAppLoop::Run()
{
	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

#endif // _WINDOWS
