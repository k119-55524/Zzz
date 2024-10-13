
#include "pch.h"
#include "gameWin.h"

using namespace Zzz;

int APIENTRY wWinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR lpCmdLine,
	_In_ int nCmdShow)
{
#ifdef _WINDOWS

	zEngine game;

	zResult res = game.Initialize();
	if (!res.isSuccess())
	{
		MessageBox(nullptr, res.GetDescription().c_str(), zStr(L"Initialize.").c_str(), MB_ICONERROR | MB_OK);
		return -1;
	}

	res = game.Run();
	if (!res.isSuccess())
	{
		MessageBox(nullptr, res.GetDescription().c_str(), zStr(L" Run.").c_str(), MB_ICONERROR | MB_OK);
		return -1;
	}

#endif

	return 0;
}
