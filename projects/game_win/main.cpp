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
	DOut("[Windows OS]. Game started.");

	try
	{
		Engine engine("GameWin_ZzzEngine");
		auto res = engine.Initialize();
		if (res)
		{

		}
		else
		{
			DOutError("[Windows OS]. Game started error: {}.", res.error());
			return -1;
		}
	}
	catch (const std::exception& e)
	{
		DOutException("[Windows OS]. Game started exception: {}.", e.what());
		return -1;
	}
	catch (...)
	{
		DOutException("[Windows OS]. Game started unknown exception.");
		return -1;
	}

	return 0;
}