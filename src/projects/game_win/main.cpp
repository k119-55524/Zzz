#include <core/Core.h>
#include <logger.h>
#include "main.h"

using namespace zzz;
using namespace zzz::core;
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

		DOut("[Main::WinMain (Windows)] - Игра запущена.");

		try
		{
			Engine engine("GameWin_ZzzEngine");
			auto res = engine.Run();
			if (!res)
			{
				DOutError("[Main::WinMain (Windows)] - Ошибка выполнения игры: {}.", res.error());
				exitCode = -1;
			}
			else
			{
				DOut("[Main::WinMain (Windows)] - Игра завершена успешно.");
			}
		}
		catch (const std::exception& e)
		{
			DOutException("[Main::WinMain (Windows)] - Исключение в WinMain: {}.", e.what());
			exitCode = -1;
		}
		catch (...)
		{
			DOutException("[Main::WinMain (Windows)] - Исключение в WinMain.");
			exitCode = -1;
		}
	}

	auto leakResult = CRT_LEAK_CHECK_END();
	return exitCode != 0 ? exitCode : leakResult;
}
