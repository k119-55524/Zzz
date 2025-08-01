#include "pch.h"
export module mlMSWin;

import IMainLoop;

namespace zzz::platforms
{
#ifdef _WIN64
	export class mlMSWin : public IMainLoop
	{
	public:
		mlMSWin() = delete;
		mlMSWin(const mlMSWin&) = delete;
		mlMSWin(mlMSWin&&) = delete;

		mlMSWin(std::function<void()> _updateSystem);

		virtual ~mlMSWin() = default;

		mlMSWin& operator=(const mlMSWin&) = delete;
		mlMSWin& operator=(mlMSWin&&) = delete;

		void Run() override;
	};

	mlMSWin::mlMSWin(std::function<void()> _updateSystem) :
		IMainLoop(_updateSystem)
	{ }

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
				updateSystem();
			}
		}
	}
#endif // _WIN64
}