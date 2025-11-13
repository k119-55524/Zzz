
#include "pch.h"
export module Engine;

import View;
import IGAPI;
import size2D;
import Colors;
import result;
import AppTime;
import zMsgBox;
import Settings;
import IMainLoop;
import ThreadPool;
import StrConvert;
import GpuEventScope;
import IOPathFactory;
import EngineFactory;
import ScenesManager;
import CPUResManager;
import GPUResManager;
import MainLoop_MSWin;
import PerformanceMeter;

using namespace zzz;
using namespace zzz::templates;
using namespace zzz::engineCore;

namespace zzz
{
#if defined(ZRENDER_API_D3D12)
	typedef MainLoop_MSWin MainLoop;
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
		std::shared_ptr<CPUResManager> m_ResCPU;
		std::shared_ptr<GPUResManager> m_ResGPU;
		std::shared_ptr<ScenesManager> m_ScenManager;
		std::shared_ptr<IGAPI> m_GAPI;
		std::shared_ptr<View> m_View;
		std::shared_ptr<IMainLoop> m_MainLoop;
		ThreadPool transferResToGPU;
		AppTime m_time;

		void Reset() noexcept;
		void OnViewResize(const size2D<>& size, eTypeWinResize resizeType);
		void OnUpdateSystem();
		void OnViewResizing();

		PerformanceMeter m_PerfRender;
	};

	Engine::Engine() :
		initState{ eInitState::InitNot },
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

		initState = eInitState::InitNot;
		isSysPaused = true;
	}

	result<> Engine::Initialize(std::wstring settingFilePath) noexcept
	{
		std::lock_guard<std::mutex> lock(stateMutex);

		if (initState != eInitState::InitNot)
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

			m_ResCPU = safe_make_shared<CPUResManager>(m_setting);
			m_ResGPU = safe_make_shared<GPUResManager>(m_GAPI, m_ResCPU);
			m_ScenManager = safe_make_shared<ScenesManager>(m_ResGPU);

			// Содаём основное окно(View) приложения
			m_View = safe_make_shared<View>(m_setting, m_ScenManager, m_GAPI);
			m_View->viewResized += std::bind(&Engine::OnViewResize, this, std::placeholders::_1, std::placeholders::_2);
			m_View->viewResizing += std::bind(&Engine::OnViewResizing, this);

			// Создаём главный цикл приложения
			m_MainLoop = safe_make_shared<MainLoop>();
			m_MainLoop->onUpdateSystem += std::bind(&Engine::OnUpdateSystem, this);

			initState = eInitState::InitOK;
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
		if (initState != eInitState::InitOK)
			return Unexpected(eResult::failure, L">>>>> [Engine::Run()]. Engine is not initialized.");

		if (initState == eInitState::Running)
			return Unexpected(eResult::failure, L">>>>> [Engine::Run()]. Engine is already running.");

		initState = eInitState::Running;
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
				.and_then([&err](const std::wstring& wstr) { err = L">>>>> [Engine::Run()]. Exception: " + wstr + L"\n"; })
				.or_else([&err](const Unexpected& error) { err = L">>>>> #0 [Engine::Run()]. Unknown exception occurred" + std::wstring(L"\n"); });
		}
		catch (...)
		{
			err = L">>>>> #1 [Engine::Run()]. Unknown exception occurred";
		}

		zMsgBox::Error(err);
		Reset();
		return Unexpected(eResult::exception, err);
	}

	void Engine::OnUpdateSystem()
	{
		if (isSysPaused)
			return;

		static zU64 frame = 0;

		static zU32 frameCount = 0;
		static double allTime = 0.0;
		m_PerfRender.StartPerformance();

		m_GAPI->BeginRender();
		{
			//GpuEventScope scope(m_GAPI, std::format(">>>>> [Engine::OnUpdateSystem()]. frame: {}", frame).c_str(), Colors::Blue);

			//Если предыдущий вызов отправки ресурсов GPU завершён и есть ресурсы для отправки в GPU, то запускаем новый
			if (transferResToGPU.IsCompleted() && m_GAPI->HasResourcesToUpload())
				transferResToGPU.Submit([&]() { m_GAPI->TranferResourceToGPU(); });

			m_View->OnUpdate(m_time.GetDeltaTime());
		}
		m_GAPI->EndRender();
		frame++;

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

		// Тестовый код для проверки переключения в полноэкранный режим и обратно
		{
			//static int frameCount = 0;
			//frameCount++;

			//if (frameCount == 2)
			//	m_View->SetVSync(true);

			//if (frameCount == 5000)
			//	m_View->SetFullScreen(true);

			//if (frameCount == 15000)
			//	m_View->SetFullScreen(false);
		}
	}

	void Engine::OnViewResizing()
	{
		OnUpdateSystem();
		OnUpdateSystem();
	}

	void Engine::OnViewResize(const size2D<>& size, eTypeWinResize resizeType)
	{
		switch (resizeType)
		{
		case eTypeWinResize::Hide:
			isSysPaused = true;
			m_time.Pause(isSysPaused);
			break;
		case eTypeWinResize::Show:
		case eTypeWinResize::Resize:
			isSysPaused = false;
			m_time.Pause(isSysPaused);
			break;
		}
	}
}