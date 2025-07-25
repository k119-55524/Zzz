
#include "pch.h"
import engine;
import result;

using namespace zzz;
using namespace zzz::result;

int APIENTRY wWinMain(
	_In_		HINSTANCE	hInstance,
	_In_opt_	HINSTANCE	hPrevInstance,
	_In_		LPWSTR		lpCmdLine,
	_In_		int			nCmdShow)
{
	zzz::engine engine;
	zResult<> res = engine.Initialize()
		.and_then([&engine](eResult) { return engine.Run(); })
		.or_else([](const Unexpected& error)
			{
				std::wcerr << L">>>>> [wWinMain( ... )]. Error: " << error.getMessage() << std::endl;
				return zResult<>(error);
			});

	return res ? 0 : 1;
}
