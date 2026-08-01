
#include <common/Common.h>

#include "main.h"

using namespace zzz;
using namespace zzz::common;
using namespace zzz::logger;
using namespace zzz::engine;

// Windows
int APIENTRY wWinMain(
	_In_		HINSTANCE	/*hInstance*/,
	_In_opt_	HINSTANCE	/*hPrevInstance*/,
	_In_		LPWSTR		/*lpCmdLine*/,
	_In_		int			/*nCmdShow*/)
{
	CRT_LEAK_CHECK_BEGIN();
#if defined(_DEBUG) && defined(_MSC_VER)
	//_CrtSetBreakAlloc(292);
#endif
	int exitCode = 0;

	{
		//g_Logger.SetLogFilterMask(eLogMessageType::All);
		//g_Logger.AddConsoleBroadcaster();
		g_Logger.AddNetworkBroadcaster(c_LocalhostIPv4, c_DefaultLoggerPort);

		DOut("[Windows OS]. Игра запущена.");

		try
		{
			Engine engine("GameWin_ZzzEngine");
			auto res = engine.Run();
			if (!res)
			{
				DOutError("[Windows OS]. Ошибка выполнения игры: {}.", res.error());
				exitCode = -1;
			}
			else
			{
				DOut("[Windows OS]. Игра завершена успешно.");
			}
		}
		catch (const std::exception& e)
		{
			DOutException("[Windows OS]. Исключение в WinMain: {}.", e.what());
			exitCode = -1;
		}
		catch (...)
		{
			DOutException("[Windows OS]. Исключение в WinMain.");
			exitCode = -1;
		}
	}

	auto leakResult = CRT_LEAK_CHECK_END();
	return exitCode != 0 ? exitCode : leakResult;
}