
#include "pch.h"
export module Engine;

import View;
import IGAPI;
import size2D;
import result;
import AppTime;
import zMsgBox;
import mlMSWin;
import Settings;
import IMainLoop;
import ThreadPool;
import strConvert;
import IOPathFactory;
import EngineFactory;
import ScenesManager;
import PerformanceMeter;
import CPUResourcesManager;
import GPUResourcesManager;

using namespace zzz;
using namespace zzz::io;
using namespace zzz::templates;
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
	export class Engine
	{
	public:
		Engine();
		~Engine();

		result<> Initialize(std::wstring settingFilePath) noexcept;
		result<> Run() noexcept;

	private:
		EngineFactory m_factory;

		eInitState initState;
		std::mutex stateMutex;
		bool isSysPaused;

		std::shared_ptr<Settings> m_setting;
		std::shared_ptr<CPUResourcesManager> m_ResCPU;
		std::shared_ptr<GPUResourcesManager> m_ResGPU;
		std::shared_ptr<ScenesManager> m_ScenManager;
		std::shared_ptr<IGAPI> m_GAPI;
		std::shared_ptr<View> m_View;
		std::shared_ptr<IMainLoop> m_MainLoop;
		ThreadPool transferResToGPU;
		AppTime m_time;

		void Reset() noexcept;
		void OnViewResized(const size2D<>& size, e_TypeWinResize resizeType);
		void OnUpdateSystem();

		PerformanceMeter m_PerfRender;
	};

	Engine::Engine() :
		initState{ eInitState::eInitNot },
		transferResToGPU{ 1 },
		isSysPaused{ true }
	{
	}

	Engine::~Engine()
	{
		Reset();
	}

	void Engine::Reset() noexcept
	{
		m_MainLoop.reset();
		m_View.reset();
		m_GAPI.reset();
		m_ScenManager.reset();
		m_ResCPU.reset();
		m_ResGPU.reset();
		m_setting.reset();

		initState = eInitState::eInitNot;
		isSysPaused = true;
	}

	result<> Engine::Initialize(std::wstring settingFilePath) noexcept
	{
		std::lock_guard<std::mutex> lock(stateMutex);

		if (initState != eInitState::eInitNot)
			return Unexpected(eResult::failure, L">>>>> [Engine::initialize()]. Re-initialization is not allowed.");

		std::wstring err;
		try
		{
			// Читаем настройки из файла
			m_setting = safe_make_shared<Settings>(settingFilePath);

			// TODO: После тип GAPI буду передавать из m_settings
			// Создаём обёртку над графическим API
			auto res = m_factory.CreateGAPI(m_setting);
			if (!res)
				return Unexpected(eResult::failure, L">>>>> [Engine::initialize()]. Failed to create GAPI.");
			m_GAPI = res.value();

			m_ResCPU = safe_make_shared<CPUResourcesManager>(m_setting);
			m_ResGPU = safe_make_shared<GPUResourcesManager>(m_GAPI, m_ResCPU);
			m_ScenManager = safe_make_shared<ScenesManager>(m_ResGPU);

			// Содаём основное окно(View) приложения
			m_View = safe_make_shared<View>(m_setting, m_ScenManager, m_GAPI, std::bind(&Engine::OnViewResized, this, std::placeholders::_1, std::placeholders::_2));

			// Создаём главный цикл приложения
			m_MainLoop = safe_make_shared<MainLoop>();
			m_MainLoop->onUpdateSystem += std::bind(&Engine::OnUpdateSystem, this);

			initState = eInitState::eInitOK;
			return {};
		}
		catch (const std::exception& e)
		{
			string_to_wstring(e.what())
				.and_then([&err](const std::wstring& wstr) { err = L">>>>> [Engine::initialize()].\n" + wstr; })
				.or_else([&err](const Unexpected& error) { err = L">>>>> #0 [Engine::initialize()]. Unknown exception occurred."; });
		}
		catch (...)
		{
			err = L">>>>> #1 [wWinMain( ... )]. Unknown exception occurred.";
		}

		zMsgBox::Error(err);
		Reset();
		return Unexpected(eResult::exception, err);
	}

	result<> Engine::Run() noexcept
	{
		std::lock_guard<std::mutex> lock(stateMutex);
		if (initState != eInitState::eInitOK)
			return Unexpected(eResult::failure, L">>>>> [Engine::go()]. Engine is not initialized.");

		if (initState == eInitState::eRunning)
			return Unexpected(eResult::failure, L">>>>> [Engine::go()]. Engine is already running.");

		initState = eInitState::eRunning;
		std::wstring err;

		try
		{
			isSysPaused = false;
			
			m_View->SetVSync(false);
			m_time.Reset();
			m_MainLoop->Run();

			return {};
		}
		catch (const std::exception& e)
		{
			string_to_wstring(e.what())
				.and_then([&err](const std::wstring& wstr) { err = L">>>>> [Engine::go()]. Exception: " + wstr + L"\n"; })
				.or_else([&err](const Unexpected& error) { err = L">>>>> #0 [Engine::go()]. Unknown exception occurred" + std::wstring(L"\n"); });
		}
		catch (...)
		{
			err = L">>>>> #1 [Engine::go()]. Unknown exception occurred";
		}

		zMsgBox::Error(err);
		Reset();
		return Unexpected(eResult::exception, err);
	}

	void Engine::OnUpdateSystem()
	{
		if (isSysPaused)
			return;

		//Sleep(100);

		//Если предыдущий вызов отправки ресурсов на GPU завершён
		if (transferResToGPU.IsCompleted() && m_GAPI->HasResourcesToUpload())
			transferResToGPU.Submit([&]() { m_GAPI->TranferResourceToGPU(); });

		static zU32 frameCount = 0;
		static double allTime = 0.0;
		m_PerfRender.StartPerformance();
		m_View->OnUpdate(m_time.GetDeltaTime());
		allTime += m_PerfRender.StopPerformance();
		{
			frameCount++;
			if (allTime >= 1.0)
			{
				double mspf = (allTime / frameCount) * 1000;;
				std::wstring cap = std::format(L".  fps: {}.  mspf: {:.4f}.", frameCount, mspf);
				m_View->AddViewCaptionText(cap);
				frameCount = 0;
				allTime = 0.0f;
			}
		}
	}

	void Engine::OnViewResized(const size2D<>& size, e_TypeWinResize resizeType)
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

		DebugOutput(std::format(L">>>>> [Engine::OnResizeAppWin()]. isSysPaused: {}", isSysPaused));
	}
}