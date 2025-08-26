
#include "pch.h"
export module engine;

import View;
import IGAPI;
import size2D;
import result;
import AppTime;
import zMsgBox;
import mlMSWin;
import Settings;
import IMainLoop;
import strConvert;
import IOPathFactory;
import engineFactory;
import ScenesManager;
import PerformanceMeter;
import ResourcesManager;

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
		engineFactory m_factory;

		eInitState initState;
		std::mutex stateMutex;
		bool isSysPaused;

		std::shared_ptr<Settings> m_setting;
		std::shared_ptr<ResourcesManager> m_resourcesManager;
		std::shared_ptr<ScenesManager> m_scenesManager;
		std::shared_ptr<IGAPI> m_GAPI;
		std::shared_ptr<View> m_view;
		std::shared_ptr<IMainLoop> mainLoop;
		AppTime m_time;

		void Reset() noexcept;
		void OnViewResized(const size2D<>& size, e_TypeWinResize resizeType);
		void OnUpdateSystem();

		PerformanceMeter m_PerfRender;
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
		m_view.reset();
		m_GAPI.reset();
		m_scenesManager.reset();
		m_resourcesManager.reset();
		m_setting.reset();

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
			// Читаем настройки из файла
			m_setting = safe_make_shared<Settings>(settingFilePath);
			// Создаем менеджер ресурсов
			m_resourcesManager = safe_make_shared<ResourcesManager>(m_setting);
			// Создаём менеджер сцен
			m_scenesManager = safe_make_shared<ScenesManager>(m_resourcesManager);

			// TODO: После тип GAPI буду передавать из m_settings
			// Создаём обёртку над графическим API
			auto res = m_factory.CreateGAPI(m_setting);
			if (!res)
				return Unexpected(eResult::failure, L">>>>> [engine::initialize()]. Failed to create GAPI.");
			m_GAPI = res.value();

			// Содаём основное окно(View) приложения
			m_view = safe_make_shared<View>(m_setting, m_scenesManager, m_GAPI, std::bind(&engine::OnViewResized, this, std::placeholders::_1, std::placeholders::_2));

			// Создаём главный цикл приложения
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
			isSysPaused = false;
			m_view->SetVSync(false);
			m_time.Reset();
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

		static zU32 frameCount = 0;
		static double allTime = 0.0;
		m_PerfRender.StartPerformance();
		m_view->OnUpdate(m_time.GetDeltaTime());
		allTime += m_PerfRender.StopPerformance();
		{
			frameCount++;
			if (allTime >= 1.0)
			{
				double mspf = (allTime / frameCount) * 1000;;
				std::wstring cap = std::format(L".  fps: {}.  mspf: {:.4f}.", frameCount, mspf);
				m_view->AddViewCaptionText(cap);
				frameCount = 0;
				allTime = 0.0f;
			}
		}
	}

	void engine::OnViewResized(const size2D<>& size, e_TypeWinResize resizeType)
	{
		switch (resizeType)
		{
		case e_TypeWinResize::eHide:
			isSysPaused = true;
			m_time.Pause(isSysPaused);
			break;
		case e_TypeWinResize::eShow:
		case e_TypeWinResize::eResize:
			isSysPaused = false;
			m_time.Pause(isSysPaused);
			break;
		}

		DebugOutput(std::format(L">>>>> [engine::OnResizeAppWin()]. isSysPaused: {}\n", isSysPaused));
	}
}