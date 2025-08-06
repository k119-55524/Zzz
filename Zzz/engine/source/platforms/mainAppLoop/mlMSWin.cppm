#include "pch.h"
export module mlMSWin;

import zEvent;
import IMainLoop;

using namespace zzz;

namespace zzz::platforms
{
#ifdef _WIN64
	export class mlMSWin : public IMainLoop
	{
	public:
		mlMSWin() = default;
		mlMSWin(const mlMSWin&) = delete;
		mlMSWin(mlMSWin&&) = delete;

		virtual ~mlMSWin() = default;

		mlMSWin& operator=(const mlMSWin&) = delete;
		mlMSWin& operator=(mlMSWin&&) = delete;

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
				//updateSystem();
			}
		}
	}
#endif // _WIN64
}