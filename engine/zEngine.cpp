
#include "pch.h"
#include "zEngine.h"

using namespace Zzz;

zEngine::zEngine() :
	engineState{ eInitNot }
{
}

zResult zEngine::Initialize(const s_zEngineInit* const initData)
{
	lock_guard<mutex> lock(initMutex);

	try
	{
		if (engineState != e_EngineState::eInitNot)
			return  zResult(e_ErrorCode::Failure);

		engineState = e_EngineState::eInitProcess;

		platform = make_unique<Platform>(initFactory.GetSystemImplementationMessageBox());

		gapi = initFactory.GetGraphicsAPI(move(initFactory.GetAplicationWindows()));
		gapi->Initialize(initData);
	}
	catch (...)
	{
		OutputDebugString(_T(">>>>> [zEngine::Initialize( ... )]. Возникло исключение при инициализации.\n"));

		return zResult(e_ErrorCode::Failure);
	}

	engineState = e_EngineState::eInitOK;

	return zResult();
}
