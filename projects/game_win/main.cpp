#include "main.h"

using namespace zzz::logger;
using namespace zzz::engine;

// Windows
int APIENTRY wWinMain(
	_In_		HINSTANCE	hInstance,
	_In_opt_	HINSTANCE	hPrevInstance,
	_In_		LPWSTR		lpCmdLine,
	_In_		int			nCmdShow)
{
	DOutLite("[Windows OS]. Game started.");

	Engine engine;
	auto res = engine.Initialize();
	if (res)
	{

	}
	else
	{
		DOut("[Windows OS]. Game started error: {}.", res.error());
		return -1;
	}

	return 0;
}