
#include "core/utils/Defines.h"

#if defined(Z_WINDOWS)

#include "engine/EngineIncludes.h"
#include "MainLoopMSWin.h"

using namespace zzz::engine;

MainLoopMSWin::MainLoopMSWin(const Platform& platform, std::function<void()> onUpdate) :
	MainLoopBase(platform, onUpdate)
{
}

void MainLoopMSWin::Run()
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

#endif // defined(Z_WINDOWS)
