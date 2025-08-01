
#include "pch.h"
export module engine;

import result;
import zSize2D;
import mlMSWin;
import IMainLoop;
import swSettings;
import ISuperWidget;
import IOPathFactory;
import StringConverters;

#if defined(_WIN64)
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
#else
#error ">>>>> [Compile error]. This branch requires implementation for the current platform"
#endif

	enum eEngineState : zU32
	{
		eInitNot,	// Готов к инициализации
		eInitOK,	// Инициализированн
		eRunning,	// Идёт процесс работы
	};

	const std::wstring swSetFolderName = L"appdata";
	const std::wstring swSetFileName = L"mui.zaml";
}

export namespace zzz
{
	export class engine
	{
	public:
		engine();
		~engine();

		zResult<> Initialize() noexcept;
		zResult<> Run() noexcept;

	private:
		eEngineState initState;
		std::mutex stateMutex;
		bool isSysPaused;

		std::shared_ptr<swSettings> settingsSW;
		std::shared_ptr<SuperWidget> superWidget;
		std::shared_ptr<IMainLoop> mainLoop;

		void Reset() noexcept;
		void OnResizeSW(const zSize2D<>& size, e_TypeWinAppResize resType);
		void OnUpdateSystem();
	};

	engine::engine() :
		initState{ eEngineState::eInitNot },
		isSysPaused{ true }
	{ }

	engine::~engine()
	{
		Reset();
	}

	void engine::Reset() noexcept
	{
		superWidget.reset();
		settingsSW.reset();

		initState = eEngineState::eInitNot;
		isSysPaused = true;
	}

	zResult<> engine::Initialize() noexcept
	{
		std::lock_guard<std::mutex> lock(stateMutex);
		if (initState != eEngineState::eInitNot)
			return Unexpected(eResult::failure, L">>>>> [engine::initialize()]. Re-initialization is not allowed.");

		std::wstring err;
		try
		{
			settingsSW = std::make_shared<swSettings>(IOPathFactory::BuildPath(IOPathFactory::GetPath_ExecutableSubfolder(), swSetFolderName, swSetFileName));
			ensure(settingsSW);
			superWidget = std::make_shared<SuperWidget>(settingsSW, std::bind(&engine::OnResizeSW, this, std::placeholders::_1, std::placeholders::_2));
			ensure(superWidget);
			auto res = superWidget->Initialize();
			if (!res)
				return Unexpected(eResult::failure, L">>>>> [engine::initialize()]. Failed to initialize super widget. More specifically: " + res.error().getMessage());

			mainLoop = std::make_shared<MainLoop>(std::bind(&engine::OnUpdateSystem, this));
			ensure(mainLoop);

			initState = eEngineState::eInitOK;
			
			return zResult<>();
		}
		catch (const std::exception& e)
		{
			string_to_wstring(e.what())
				.and_then([&err](const std::wstring& wstr) { err = L">>>>> [engine::initialize()]. Exception: " + wstr + L"\n"; })
				.or_else([&err](const Unexpected& error) { err = L">>>>> #0 [engine::initialize()]. Unknown exception occurred\n"; });
		}
		catch (...)
		{
			err = L">>>>> #1 [wWinMain( ... )]. Unknown exception occurred\n";
		}

		Reset();
		return Unexpected(eResult::exception, err);
	}

	zResult<> engine::Run() noexcept
	{
		std::lock_guard<std::mutex> lock(stateMutex);
		if (initState != eEngineState::eInitOK)
			return Unexpected(eResult::failure, L">>>>> [engine::go()]. Engine is not initialized.");

		if (initState == eEngineState::eRunning)
			return Unexpected(eResult::failure, L">>>>> [engine::go()]. Engine is already running.");

		initState = eEngineState::eRunning;
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
			DebugOutput(L">>>>> [engine::OnResizeSW]. Show app window.\n");

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