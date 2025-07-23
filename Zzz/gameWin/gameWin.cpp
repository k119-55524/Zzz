
#include "pch.h"
import engine;
#include "gameWin.h"

int APIENTRY wWinMain(
	_In_		HINSTANCE	hInstance,
	_In_opt_	HINSTANCE	hPrevInstance,
	_In_		LPWSTR		lpCmdLine,
	_In_		int			nCmdShow)
{
	zzz::engine engine;
	auto res = engine.initialize();
	if (res)
		engine.go();

	return (int)0;
}
