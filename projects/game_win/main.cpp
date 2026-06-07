#include "main.h"

using namespace zzz;
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
	// Чтобы не засорять вывод компилятора
	(void)hInstance;
	(void)hPrevInstance;
	(void)lpCmdLine;
	(void)nCmdShow;

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
				return -1;
			}
			else
			{
				DOut("[Windows OS]. Game exited successfully.");
			}
		}
		else
		{
			DOutError("[Windows OS]. Game started error: {}.", res.error());
			return -1;
		}
	}
	catch (const std::exception& e)
	{
		//DOutException("[Windows OS]. WinMain {}.", e.what());
		return -1;
	}
	catch (...)
	{
		//DOutException("[Windows OS]. WinMain exception.");
		return -1;
	}

	return 0;
}