#include "MainLoop_MSWin.h"


using namespace zzz::engine;

MainLoop_MSWin::MainLoop_MSWin(const std::shared_ptr<Platform> platform) :
	MainLoopBase(platform)
{
}

void MainLoop_MSWin::Run()
{
	MSG msg = { 0 };

	while (isRunning)//msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				isRunning = false;
				break;
			}

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			onUpdateSystem();
		}
	}
}
