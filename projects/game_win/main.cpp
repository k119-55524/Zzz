
#include <common/common.h>

#include "main.h"

using namespace zzz;
using namespace zzz::common;
using namespace zzz::logger;
using namespace zzz::engine;

// Windows
int APIENTRY wWinMain(
	_In_		HINSTANCE	hInstance,
	_In_opt_	HINSTANCE	hPrevInstance,
	_In_		LPWSTR		lpCmdLine,
	_In_		int			nCmdShow)
{
	// Подавляем предупреждения о неиспользуемых параметрах
	(void)hInstance;
	(void)hPrevInstance;
	(void)lpCmdLine;
	(void)nCmdShow;

	CRT_LEAK_CHECK_BEGIN();
	int exitCode = 0;

	{
		//Engine::SetLogFilterMask(eLogMessageType::All);
		//Engine::AddConsoleBroadcaster();
		Engine::AddNetworkBroadcaster(c_LocalhostIPv4, 3030);

		DOut("[Windows OS]. Game started.");

		try
		{
			Engine engine("GameWin_ZzzEngine");
			auto res = engine.Initialize();
			if (res)
			{
				res = engine.Run();
				if (!res)
				{
					DOutError("[Windows OS]. Game runtime error: {}.", res.error());
					exitCode = -1;
				}
				else
				{
					DOut("[Windows OS]. Game exited successfully.");
				}
			}
			else
			{
				DOutError("[Windows OS]. Game started error: {}.", res.error());
				exitCode = -1;
			}
		}
		catch (const std::exception& e)
		{
			DOutException("[Windows OS]. WinMain {}.", e.what());
			exitCode = -1;
		}
		catch (...)
		{
			DOutException("[Windows OS]. WinMain exception.");
			exitCode = -1;
		}
	}

	auto leakResult = CRT_LEAK_CHECK_END();
	return exitCode != 0 ? exitCode : leakResult;
}