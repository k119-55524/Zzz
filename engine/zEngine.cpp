﻿
#include "pch.h"
#include "zEngine.h"

using namespace Zzz;

zEngine::zEngine() :
	initState{ eInitNot },
	isAppPaused{true}
{
}

zEngine::~zEngine()
{
	if (userGS != nullptr)
		userGS->Save();
}

zResult zEngine::Initialize()
{
	lock_guard<mutex> lock(initMutex);

	zResult res;

	try
	{
		if (initState != e_InitState::eInitNot)
			return  zResult(e_ErrorCode::eFailure, L">>>>> [zEngine::Initialize()]. Attempting to reinitialize.");

		Reset(e_InitState::eInitProcess);

		platform = make_unique<Platform>(factoryPlatform.GetSystemImplementationMessageBox());
		platformIO = factoryPlatform.GetPlatformIO();

		userGS = make_shared<UserGameSettings>(platformIO);
		if (userGS == nullptr)
			throw runtime_error(">>>>> [zEngine::Initialize( ... )]. Failed to allocate memory for UserGameSettings.");

		res = userGS->Load();
		if (!res.isSuccess())
		{
			Reset();
			return res;
		}

		appWin = factoryInit.GetAplicationWindows(bind(&zEngine::OnResizeWindow, this, placeholders::_1, placeholders::_2));
		appWin->Initialize(userGS);

		gAPI = factoryInit.GetGraphicsAPI(appWin);
		gAPI->Initialize();

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

void zEngine::Reset(e_InitState state)
{
	appWin.reset();
	gAPI.reset();
	mainLoop.reset();
	platform.reset();
	platformIO.reset();
	userGS.reset();

	initState = state;
}

void zEngine::OnUpdateSystem()
{
	if (isAppPaused)
		return;

	//Sleep(100);

	sceneManager.Update();
	gAPI->Update();
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
