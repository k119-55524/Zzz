#include "pch.h"
export module mlMSWin;

import event;
import IMainLoop;

using namespace zzz;

namespace zzz::platforms
{
#if defined(ZPLATFORM_MSWINDOWS)
	export class mlMSWin final: public IMainLoop
	{
	public:
		mlMSWin() = default;
		mlMSWin(const mlMSWin&) = delete;
		mlMSWin(mlMSWin&&) = delete;

		mlMSWin& operator=(const mlMSWin&) = delete;
		mlMSWin& operator=(mlMSWin&&) = delete;

		virtual ~mlMSWin() = default;

		void Run() override;
	};

	void mlMSWin::Run()
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