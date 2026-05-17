#include "pch.h"

import engine;
import logger;

#include "modules/main.h"

using namespace zzz::logger;
using namespace zzz::engine;

int APIENTRY wWinMain(
	_In_		HINSTANCE	hInstance,
	_In_opt_	HINSTANCE	hPrevInstance,
	_In_		LPWSTR		lpCmdLine,
	_In_		int			nCmdShow)
{
	DOut(L"Game started.");

	Engine engine;
	engine.Initialize();

	return 0;
}