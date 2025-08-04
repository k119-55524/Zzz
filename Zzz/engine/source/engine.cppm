
#include "pch.h"
export module engine;

import IGAPI;
import result;
import zSize2D;
import mlMSWin;
import IMainLoop;
import swSettings;
import ISuperWidget;
import IOPathFactory;
import StringConverters;

#if defined(_WIN64)
import DXAPI;
import swMSWin;
#else
#error ">>>>> [Compile error]. This branch requires implementation for the current platform"
#endif

using namespace zzz;
using namespace zzz::io;
using namespace zzz::result;
using namespace zzz::platforms;

namespace zzz
{
#if defined(_WIN64)
	typedef swMSWin SuperWidget;
	typedef mlMSWin MainLoop;
	typedef DXAPI GAPI;
#else
#error ">>>>> [Compile error]. This branch requires implementation for the current platform"
#endif

	//enum eEngineState : zU32
	//{
	//	eInitNot,	// Готов к инициализации
	//	eInitOK,	// Инициализированн
	//	eRunning,	// Идёт процесс работы
	//};
}

export namespace zzz
{
	export class engine
	{
	public:
		engine();
		~engine();

		zResult<> Initialize(std::wstring settingFilePath) noexcept;
		zResult<> Run() noexcept;

	private:
		eInitState initState;
		std::mutex stateMutex;
		bool isSysPaused;

		std::shared_ptr<swSettings> settingsSW;
		std::shared_ptr<SuperWidget> superWidget;
		std::shared_ptr<IMainLoop> mainLoop;
		std::shared_ptr<IGAPI> gapi;

		void Reset() noexcept;
		void OnResizeSW(const zSize2D<>& size, e_TypeWinAppResize resType);
		void OnUpdateSystem();
	};

	engine::engine() :
		initState{ eInitState::eInitNot },
		isSysPaused{ true }
	{ }

	engine::~engine()
	{
		Reset();
	}

	void engine::Reset() noexcept
	{
		gapi.reset();
		mainLoop.reset();
		superWidget.reset();
		settingsSW.reset();

		initState = eInitState::eInitNot;
		isSysPaused = true;
	}

	zResult<> engine::Initialize(std::wstring settingFilePath) noexcept
	{
		std::lock_guard<std::mutex> lock(stateMutex);
		if (initState != eInitState::eInitNot)
			return Unexpected(eResult::failure, L">>>>> [engine::initialize()]. Re-initialization is not allowed.");

		std::wstring err;
		try
		{
			ensure(settingsSW = std::make_shared<swSettings>(settingFilePath));
			ensure(superWidget = std::make_shared<SuperWidget>(settingsSW, std::bind(&engine::OnResizeSW, this, std::placeholders::_1, std::placeholders::_2)));
			auto res = superWidget->Initialize();
			if (!res)
				return Unexpected(eResult::failure, L">>>>> [engine::initialize()]. Failed to initialize super widget. More specifically: " + res.error().getMessage());

			ensure(mainLoop = std::make_shared<MainLoop>(std::bind(&engine::OnUpdateSystem, this)));
			ensure(gapi = std::make_shared<GAPI>(superWidget));
			res = gapi->Initialize();
			if (!res)
				return Unexpected(eResult::failure, L">>>>> [engine::initialize()]. Failed to initialize gapi. More specifically: " + res.error().getMessage());

			initState = eInitState::eInitOK;
			return zResult<>();
		}
		catch (const std::exception& e)
		{
			string_to_wstring(e.what())
				.and_then([&err](const std::wstring& wstr) { err = L">>>>> [engine::initialize()].\n" + wstr; })
				.or_else([&err](const Unexpected& error) { err = L">>>>> #0 [engine::initialize()]. Unknown exception occurred."; });
		}
		catch (...)
		{
			err = L">>>>> #1 [wWinMain( ... )]. Unknown exception occurred.";
		}

		Reset();
		return Unexpected(eResult::exception, err);
	}

	zResult<> engine::Run() noexcept
	{
		std::lock_guard<std::mutex> lock(stateMutex);
		if (initState != eInitState::eInitOK)
			return Unexpected(eResult::failure, L">>>>> [engine::go()]. Engine is not initialized.");

		if (initState == eInitState::eRunning)
			return Unexpected(eResult::failure, L">>>>> [engine::go()]. Engine is already running.");

		initState = eInitState::eRunning;
		std::wstring err;

		try
		{
			mainLoop->Run();

			return {};
		}
		catch (const std::exception& e)
		{
			string_to_wstring(e.what())
				.and_then([&err](const std::wstring& wstr) { err = L">>>>> [engine::go()]. Exception: " + wstr + L"\n"; })
				.or_else([&err](const Unexpected& error) { err = L">>>>> #0 [engine::go()]. Unknown exception occurred" + std::wstring(L"\n"); });
		}
		catch (...)
		{
			err = L">>>>> #1 [engine::go()]. Unknown exception occurred";
		}

		Reset();
		return Unexpected(eResult::exception, err);
	}

	void engine::OnUpdateSystem()
	{
		if (isSysPaused)
			return;

		//Sleep(100);

		//sceneManager->Update();
		//gAPI->Update();
	}

	void engine::OnResizeSW(const zSize2D<>& size, e_TypeWinAppResize resizeType)
	{
		switch (resizeType)
		{
		case e_TypeWinAppResize::eHide:
			DebugOutput(L">>>>> [engine::OnResizeSW]. Hide app window.\n");

			isSysPaused = true;
			break;
		case e_TypeWinAppResize::eShow:
			DebugOutput((L">>>>> [engine::OnResizeSW]. Show app window. Win size: " + std::to_wstring(size.width) + L"x" + std::to_wstring(size.height) + L".\n").c_str());

			isSysPaused = false;
			break;
		case e_TypeWinAppResize::eResize:
			DebugOutput((L">>>>> [engine::OnResizeSW]. Resize to " + std::to_wstring(size.width) + L"x" + std::to_wstring(size.height) + L".\n").c_str());

			isSysPaused = false;
			//gAPI->Resize(size);
			break;
		}
	}
}