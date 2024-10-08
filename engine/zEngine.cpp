﻿
#include "pch.h"
#include "zEngine.h"

using namespace Zzz;

zEngine::zEngine() :
	initState{ eInitNot },
	isAppPaused{true}
{
}

zResult zEngine::Initialize(const DataEngineInitialization initData)
{
	lock_guard<mutex> lock(initMutex);

	zResult res;

	try
	{
		if (initState != e_InitState::eInitNot)
			return  zResult(e_ErrorCode::eFailure, L">>>>> [zEngine::Initialize()]. Attempting to reinitialize.");

		initState = e_InitState::eInitProcess;
		initData.TestParameters();
		Reset(e_InitState::eInitProcess);

		platform = make_shared<Platform>(factoryPlatform.GetSystemImplementationMessageBox());

		appWin = factoryInit.GetAplicationWindows(bind(&zEngine::OnResizeWindow, this, placeholders::_1, placeholders::_2));
		res = appWin->Initialize(initData);
		if (!res.isSuccess())
		{
			Reset();

			return res;
		}

		gAPI = factoryInit.GetGraphicsAPI();
		res = gAPI->Initialize(initData);
		if (!res.isSuccess())
		{
			Reset();

			return res;
		}

		mainLoop = factoryInit.GetMainAppLoop(bind(&zEngine::OnUpdateSystem, this));
	}
	catch (const exception& ex)
	{
		Reset();

		zStr errorMsg = L">>>>> [zEngine::Initialize( ... )]. --- EXEPTION --- :\n+---" + zStr(ex.what(), ex.what() + strlen(ex.what())) + L"\n";
		return zResult(e_ErrorCode::eFailure, errorMsg);
	}

	initState = e_InitState::eInitOK;

	return res;
}

zResult zEngine::Run()
{
	lock_guard<mutex> lock(runMutex);

	if (initState != e_InitState::eInitOK)
		return  zResult(e_ErrorCode::eFailure, L">>>>> [zEngine::Run()]. Attempt to execute an uninitialized application.");

	try
	{
		mainLoop->Run();
	}
	catch (const exception& ex)
	{
		Reset();

		zStr errorMsg = L">>>>> [zEngine::Run( ... )]. --- EXEPTION --- :\n+---" + zStr(ex.what(), ex.what() + strlen(ex.what())) + L"\n";
		return zResult(e_ErrorCode::eFailure, errorMsg);
	}

	return zResult();
}

void zEngine::OnUpdateSystem()
{
	if (isAppPaused)
		return;

	Sleep(100);

	gAPI->Update();
	//DebugOutput(L">>>>> [zEngine::OnUpdateSystem()]. ...\n");
}

void zEngine::OnResizeWindow(const zSize& size, e_TypeWinAppResize resizeType)
{
	switch (resizeType)
	{
	case e_TypeWinAppResize::eHide:
		DebugOutput(L">>>>> [zEngine::OnResizeWindow()]. Hide app window.\n");

		isAppPaused = true;
		break;
	case e_TypeWinAppResize::eShow:
		DebugOutput(L">>>>> [zEngine::OnResizeWindow()]. Show app window.\n");

		isAppPaused = false;
		break;
	case e_TypeWinAppResize::eResize:
		DebugOutput((L">>>>> [zEngine::OnResizeWindow()]. Resize to " + to_wstring(size.width) + L"x" + to_wstring(size.height) + L".\n").c_str());

		isAppPaused = false;
		gAPI->Resize(size);
		break;
	}
}

void zEngine::Reset(e_InitState state)
{
	platform.reset();
	appWin.reset();
	gAPI.reset();
	mainLoop.reset();
	initState = state;
}
