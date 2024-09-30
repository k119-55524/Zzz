
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

		platform = make_shared<Platform>(initFactory.GetSystemImplementationMessageBox());

		gapi = initFactory.GetGraphicsAPI(move(initFactory.GetAplicationWindows()));
		res = gapi->Initialize(initData);
	}
	catch (const exception& ex)
	{
		initState = e_InitState::eInitNot;
		platform.reset();
		gapi.reset();

		zStr errorMsg = L">>>>> [zEngine::Initialize( ... )]. --- EXEPTION --- :\n" + zStr(ex.what(), ex.what() + strlen(ex.what())) + L"\n";
		return zResult(e_ErrorCode::Failure, errorMsg);
	}

	initState = e_InitState::eInitOK;

	return res;
}
