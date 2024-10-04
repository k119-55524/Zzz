
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

	InitWindowsData initWinData(L"zGameWinClass", L"zGame");
	DataEngineInitialization initData(initWinData, zSize(88, 600));

	zResult res = game.Initialize(initData);
	if (!res.isSuccess())
	{
		OutputDebugString(res.GetDescription().c_str());
		return -1;
	}

#endif

	return 0;
}
