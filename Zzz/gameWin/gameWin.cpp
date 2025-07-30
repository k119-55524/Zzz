
#include "pch.h"
import engine;
import result;
import zMsgBox;

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
		.and_then([&engine]() { return engine.Run(); })
		.or_else([](const Unexpected& error)
			{
				zMsgBox::Error(error.getMessage());
				return zResult<>(error);
			});

	return res ? 0 : static_cast<int>(res.error().getCode());
}
