
#include "source/TestManager.h"
#include "sourceTests/headerTests.h"

import Engine;

using namespace zzz::ztests;

int main()
{
#if _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//_CrtSetBreakAlloc(158);
	//_CrtSetBreakAlloc(160);
	//_CrtSetBreakAlloc(202);
#endif

	{
		//zzz::Engine Engine;
		//auto err = Engine.Initialize();
	}

#if _DEBUG
	_CrtDumpMemoryLeaks();
#endif
}
