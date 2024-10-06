
#include "Header.h"
#include "TestManager.h"
#include "Tests/Test_Engine.h"
#include "Tests/Test_zColor.h"
//#include "TestMath/TestMath_zVec2.h"
//#include "TestTemplates/Test_zVectorPtr.h"

using namespace Zzz;

int main()
{
#ifdef _SERVICES

#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//_CrtSetBreakAlloc(158);
	//_CrtSetBreakAlloc(160);
	//_CrtSetBreakAlloc(202);
#endif // _DEBUG

	{
		TestManager manager;
		manager.AddTest(make_unique<Test_zColor>());
		manager.AddTest(make_unique<Test_Engine>());
		//manager.AddTest(make_unique<TestMath_zvec2>());
		manager.RunTests();
	}

#ifdef _DEBUG
	_CrtDumpMemoryLeaks();
#endif // _DEBUG

#endif // _SERVICES

	return 0;
}