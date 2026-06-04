#include "MainLoop_MSWin.h"

#if defined(Z_WINDOWS)

using namespace zzz::engine;

void MainLoop_MSWin::Run()
{
	MSG msg = { 0 };

	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			onUpdateSystem();
		}
	}
}
#endif // defined(Z_WINDOWS)