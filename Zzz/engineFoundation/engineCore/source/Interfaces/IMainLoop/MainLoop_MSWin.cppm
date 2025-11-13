#include "pch.h"
export module MainLoop_MSWin;

import Event;
import IMainLoop;

using namespace zzz::engineCore;

namespace zzz
{
#if defined(ZPLATFORM_MSWINDOWS)
	export class MainLoop_MSWin final: public IMainLoop
	{
		Z_NO_COPY_MOVE(MainLoop_MSWin);

	public:
		MainLoop_MSWin() = default;
		virtual ~MainLoop_MSWin() = default;

		void Run() override;
	};

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
#endif // ZPLATFORM_MSWINDOWS
}