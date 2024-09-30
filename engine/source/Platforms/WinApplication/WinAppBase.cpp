#include "pch.h"
#include "WinAppBase.h"

using namespace Zzz;
using namespace Zzz::Platforms;

WinAppBase::WinAppBase() :
	initState{ e_InitState::eInitNot }
{
}

zResult WinAppBase::Initialize(const s_zEngineInit::WinAppSettings* data)
{
	lock_guard<mutex> lock(initMutex);

	if (initState != e_InitState::eInitNot)
		throw runtime_error(">>>>> [WinAppBase::WinAppBase()]. Attempting to reinitialize.");

	initState = e_InitState::eInitProcess;

	zResult res = Init(data);

	initState = res.isSuccess() ? e_InitState::eInitOK : e_InitState::eInitError;

	return res;
}
