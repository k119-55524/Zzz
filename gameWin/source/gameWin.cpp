
#include "pch.h"
#include "gameWin.h"

using namespace Zzz;

int APIENTRY wWinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR lpCmdLine,
	_In_ int nCmdShow)
{
	zEngine game;

	InitMSWindowsData initWinData(L"zGameWinClass");
	s_zEngineInit initData(initWinData);

	zResult res = game.Initialize(initData);
	if (!res.isSuccess())
	{
		OutputDebugString(res.GetDescription().c_str());
		return -1;
	}

	return 0;
}
