#include "MainLoop_MSWin.h"


using namespace zzz::engine;

MainLoop_MSWin::MainLoop_MSWin(const Platform& platform, std::function<void()> onUpdate) :
	MainLoopBase(platform, onUpdate)
{
}

void MainLoop_MSWin::Run()
{
	MSG msg = { 0 };

	while (isRunning)
	{
		while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				isRunning = false;
				break;
			}

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		
		if (isRunning)
			OnUpdate();
	}
}
