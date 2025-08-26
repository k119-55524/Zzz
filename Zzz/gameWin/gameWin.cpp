
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
#ifdef _DEBUG
		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
		//_CrtSetBreakAlloc(263);
		//_CrtSetBreakAlloc(160);
		//_CrtSetBreakAlloc(202);
#endif // _DEBUG
	{
		result<> res;
		zzz::engine engine;
		res = engine.Initialize(L".\\appdata\\ui.zaml")
			.and_then([&engine]() { return engine.Run(); });
	}

#ifdef _DEBUG
	_CrtDumpMemoryLeaks();
#endif // _DEBUG

	return 0;
}
