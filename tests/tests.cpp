
#include "Header.h"
#include "TestManager.h"
#include "Tests/Test_Engine.h"
//#include "TestMath/TestMath_zVec2.h"
//#include "TestTemplates/Test_zVectorPtr.h"

using namespace Zzz;

int main()
{
#if _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//_CrtSetBreakAlloc(158);
	//_CrtSetBreakAlloc(160);
	//_CrtSetBreakAlloc(202);
#endif

	{
		TestManager manager;
		//manager.AddTest(make_unique<Test_zVectorPtr>());
		manager.AddTest(make_unique<Test_Engine>());
		//manager.AddTest(make_unique<TestMath_zvec2>());
		manager.RunTests();
	}

#if _DEBUG
	_CrtDumpMemoryLeaks();
#endif

	return 0;
}