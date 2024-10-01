
#include "pch.h"
#include "zEngine.h"

using namespace Zzz;

zEngine::zEngine() :
	initState{ eInitNot }
{
}

zResult zEngine::Initialize(const s_zEngineInit* const initData)
{
	lock_guard<mutex> lock(initMutex);

	zResult res;

	try
	{
		if (initState != e_InitState::eInitNot)
			return  zResult(e_ErrorCode::Failure, L">>>>> [zEngine::Initialize()]. Attempting to reinitialize.");

		initState = e_InitState::eInitProcess;

		platform.reset();
		mainLoop.reset();

		platform = make_shared<Platform>(factoryPlatform.GetSystemImplementationMessageBox());
		mainLoop = factoryInit.GetMainAppLoop(factoryInit.GetAplicationWindows(), factoryInit.GetGraphicsAPI());

		res = mainLoop->Initialize(initData);
	}
	catch (const exception& ex)
	{
		platform.reset();
		mainLoop.reset();
		initState = e_InitState::eInitNot;

		zStr errorMsg = L">>>>> [zEngine::Initialize( ... )]. --- EXEPTION --- :\n" + zStr(ex.what(), ex.what() + strlen(ex.what())) + L"\n";
		return zResult(e_ErrorCode::Failure, errorMsg);
	}

	initState = e_InitState::eInitOK;

	return res;
}
