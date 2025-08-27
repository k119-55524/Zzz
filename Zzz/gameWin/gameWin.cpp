
#include "pch.h"
import Engine;
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
		zzz::Engine Engine;
		res = Engine.Initialize(L".\\appdata\\ui.zaml")
			.and_then([&Engine]() { return Engine.Run(); });
	}

#ifdef _DEBUG
	_CrtDumpMemoryLeaks();
#endif // _DEBUG

	return 0;
}
