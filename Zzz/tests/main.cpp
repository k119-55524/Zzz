
#include "source/TestManager.h"
#include "sourceTests/headerTests.h"

//import Engine;

using namespace zzz::ztests;

int main()
{
	SetConsoleOutputCP(CP_UTF8);

#if _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//_CrtSetBreakAlloc(158);
#endif

	{
		//zzz::Engine Engine;

		TestManager manager;
		manager.AddTest(std::make_unique<test_Vector3>());
		manager.AddTest(std::make_unique<test_Vector4>());
		manager.AddTest(std::make_unique<test_Quaternion>());
		manager.AddTest(std::make_unique<test_Matrix4x4>());
		manager.RunTests();
	}

#if _DEBUG
	_CrtDumpMemoryLeaks();
#endif

	return 0;
}
