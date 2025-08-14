
#include "pch.h"
export module engine;

import IGAPI;
import zView;
import result;
import zMsgBox;
import zSize2D;
import mlMSWin;
import IMainLoop;
import strConver;
import zViewSettings;
import IOPathFactory;

using namespace zzz;
using namespace zzz::io;
using namespace zzz::platforms;

namespace zzz
{
#if defined(_WIN64)
	typedef mlMSWin MainLoop;
#else
#error ">>>>> [Compile error]. This branch requires implementation for the current platform"
#endif
}

export namespace zzz
{
	export class engine
	{
	public:
		engine();
		~engine();

		result<> Initialize(std::wstring settingFilePath) noexcept;
		result<> Run() noexcept;

	private:
		eInitState initState;
		std::mutex stateMutex;
		bool isSysPaused;

		std::shared_ptr<zViewSettings> settingsView;
		std::shared_ptr<zView> view;
		std::shared_ptr<IMainLoop> mainLoop;

		void Reset() noexcept;
		void OnViewResized(const zSize2D<>& size, e_TypeWinResize resizeType);
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
		mainLoop.reset();
		view.reset();
		settingsView.reset();

		initState = eInitState::eInitNot;
		isSysPaused = true;
	}

	result<> engine::Initialize(std::wstring settingFilePath) noexcept
	{
		std::lock_guard<std::mutex> lock(stateMutex);
		if (initState != eInitState::eInitNot)
			return Unexpected(eResult::failure, L">>>>> [engine::initialize()]. Re-initialization is not allowed.");

		std::wstring err;
		try
		{
			settingsView = safe_make_shared<zViewSettings>(settingFilePath);
			view = safe_make_shared<zView>(settingsView, std::bind(&engine::OnViewResized, this, std::placeholders::_1, std::placeholders::_2));

			mainLoop = safe_make_shared<MainLoop>();
			mainLoop->onUpdateSystem += std::bind(&engine::OnUpdateSystem, this);

			initState = eInitState::eInitOK;
			return {};
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

		zMsgBox::Error(err);
		Reset();
		return Unexpected(eResult::exception, err);
	}

	result<> engine::Run() noexcept
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

		zMsgBox::Error(err);
		Reset();
		return Unexpected(eResult::exception, err);
	}

	void engine::OnUpdateSystem()
	{
		if (isSysPaused)
			return;

		//Sleep(100);

		view->OnUpdate();
	}

	void engine::OnViewResized(const zSize2D<>& size, e_TypeWinResize resizeType)
	{
		switch (resizeType)
		{
		case e_TypeWinResize::eHide:
			isSysPaused = true;
			break;
		case e_TypeWinResize::eShow:
		case e_TypeWinResize::eResize:
			isSysPaused = false;
			break;
		}

		DebugOutput(std::format(L">>>>> [engine::OnResizeAppWin()]. isSysPaused: {}\n", isSysPaused));
	}
}