
#include "pch.h"
import engine;
import result;
import StringConverters;

using namespace zzz;
using namespace zzz::result;

int APIENTRY wWinMain(
	_In_		HINSTANCE	hInstance,
	_In_opt_	HINSTANCE	hPrevInstance,
	_In_		LPWSTR		lpCmdLine,
	_In_		int			nCmdShow)
{
	try
	{
		zzz::engine engine;

		zResult<> res = engine.initialize()
			.and_then([&engine](eResult) { return engine.go(); })
			.or_else([](const Unexpected& error)
				{
					std::wcerr << L">>>>> [wWinMain( ... )]. Error: " << error.getMessage() << std::endl;
					return zResult<>(error);
				});

		return res ? 0 : 1;
	}
	catch (const std::exception& e)
	{
		zResult<std::wstring> res = string_to_wstring(e.what())
			.and_then([](const std::wstring& wstr)	{ std::wcerr << L">>>>> #0 [wWinMain( ... )]. Exception: " << wstr << std::endl; })
			.or_else ([](const Unexpected& error)	{ std::wcerr << L">>>>> [wWinMain( ... )]. Unknown exception occurred" << std::endl; });
		return 1;
	}
	catch (...)
	{
		std::wcerr << L">>>>> [wWinMain( ... )]. Unknown exception occurred" << std::endl;
		return 1;
	}
}
