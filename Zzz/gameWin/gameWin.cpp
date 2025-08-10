
#include "pch.h"
import engine;
import result;

using namespace zzz;

int APIENTRY wWinMain(
	_In_		HINSTANCE	hInstance,
	_In_opt_	HINSTANCE	hPrevInstance,
	_In_		LPWSTR		lpCmdLine,
	_In_		int			nCmdShow)
{
	zzz::engine engine;
	result<> res = engine.Initialize(L".\\appdata\\ui.zaml")
		.and_then([&engine]() { return engine.Run(); });

	return res ? 0 : static_cast<int>(res.error().getCode());
}
