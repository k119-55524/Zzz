#include "pch.h"
export module MainLoop_MSWin;

import event;
import IMainLoop;

using namespace zzz::engineCore;

namespace zzz
{
#if defined(ZPLATFORM_MSWINDOWS)
	export class MainLoop_MSWin final: public IMainLoop
	{
	public:
		MainLoop_MSWin() = default;
		MainLoop_MSWin(const MainLoop_MSWin&) = delete;
		MainLoop_MSWin(MainLoop_MSWin&&) = delete;

		MainLoop_MSWin& operator=(const MainLoop_MSWin&) = delete;
		MainLoop_MSWin& operator=(MainLoop_MSWin&&) = delete;

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