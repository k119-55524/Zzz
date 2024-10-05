#include "pch.h"
#include "WindowsMainAppLoop.h"

using namespace Zzz::Platforms;

#ifdef _WINDOWS

WindowsMainAppLoop::WindowsMainAppLoop(function<void()> _updateSystem) :
	IMainAppLoop(_updateSystem)
{
}

void WindowsMainAppLoop::Run()
{
	MSG msg = {0};

	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			if (updateSystem != nullptr)
				updateSystem();
		}
	}
}

#endif // _WINDOWS
