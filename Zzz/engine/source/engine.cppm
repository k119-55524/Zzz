
export module Engine;

export import Result;
export import UserView;
export import UserScene;
export import Transform;
export import UserLayer3D;
export import UserSceneEntity;

import View;
import IGAPI;
import Size2D;
import Colors;
import AppTime;
import zMsgBox;
import IMainLoop;
import ThreadPool;
import StrConvert;
import ZamlConfig;
import ZamlParser;
import StartupConfig;
import GpuEventScope;
import IOPathFactory;
import EngineFactory;
import CPUResManager;
import GPUResManager;
import MainLoop_MSWin;
import PerformanceMeter;
import SceneEntityFactory;

using namespace zzz;
using namespace zzz::core;
using namespace zzz::templates;

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

		[[nodiscard]] Result<> Initialize(std::wstring zamlPath) noexcept;
		[[nodiscard]] Result<> Run() noexcept;

		[[nodiscard]] std::shared_ptr<UserView> GetMainView() { return m_UserView; }

	private:
		EngineFactory m_EngineFactory;
		std::shared_ptr<ZamlConfig> m_ZamlStartupConfig;
		std::unique_ptr<StartupConfig> m_Config;

		eInitState initState;
		std::mutex stateMutex;
		bool isSysPaused;

		std::shared_ptr<CPUResManager> m_ResCPU;
		std::shared_ptr<GPUResManager> m_ResGPU;
		std::shared_ptr<SceneEntityFactory> m_EntityFactory;
		std::shared_ptr<IGAPI> m_GAPI;
		std::shared_ptr<View> m_View;
		std::shared_ptr<IMainLoop> m_MainLoop;

		std::shared_ptr<UserView> m_UserView;

		ThreadPool transferResToGPU;
		AppTime m_time;

		[[nodiscard]] Result<> Initialize() noexcept;

		void Reset() noexcept;
		void OnViewResize(const Size2D<>& size, eTypeWinResize resizeType);
		void OnUpdateSystem();
		void OnViewResizing();

		Result<std::unique_ptr<StartupConfig>> GetStartupConfig();

		PerformanceMeter m_PerfRender;
	};

	Engine::Engine() :
		initState{ eInitState::InitNot },
		transferResToGPU{std::string("Engine"), 1},
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
		m_EntityFactory.reset();
		m_GAPI.reset();
		m_ResCPU.reset();
		m_ResGPU.reset();
		m_ZamlStartupConfig.reset();

		initState = eInitState::InitNot;
		isSysPaused = true;
	}

	// Инициализация с использованием ZAML файла
	Result<> Engine::Initialize(std::wstring zamlPath) noexcept
	{
		std::lock_guard<std::mutex> lock(stateMutex);

		if (initState != eInitState::InitNot)
			return Unexpected(eResult::failure, std::format(L">>>>> [Engine::initialize({})]. Re-initialization is not allowed.", zamlPath));

		std::wstring err;
		try
		{
			// Читаем настройки из файла
			m_ZamlStartupConfig = safe_make_shared<ZamlConfig>(zamlPath);
			auto res = GetStartupConfig();
			if (!res)
				return Unexpected(eResult::failure, std::format(L">>>>> [Engine::initialize({})]. Failed to transfer settings from file.", zamlPath));
			m_Config = std::move(res.value());

			return Initialize();
		}
		catch (const std::exception& e)
		{
			auto result = string_to_wstring(e.what());

			if (result)
				err = std::format(L">>>>> [Engine::initialize({})].\n{}", zamlPath, result.value());
			else
				err = std::format(L">>>>> [Engine::initialize({})]. Unknown exception occurred.", zamlPath);
		}
		catch (...)
		{
			err = L">>>>> #1 [wWinMain( ... )]. Unknown exception occurred.";
		}

		zMsgBox::Error(err);
		Reset();
		return Unexpected(eResult::exception, err);
	}

	Result<> Engine::Initialize() noexcept
	{
		// Создаём обёртку над графическим API
		auto res = m_EngineFactory.CreateGAPI(m_Config->GetGAPIConfig());
		if (!res)
			return Unexpected(eResult::failure, L">>>>> [Engine::initialize()]. Failed to create GAPI.");
		m_GAPI = res.value();

		// Создаём менеджеры ресурсов и фабрику сущностей сцены
		m_ResCPU = safe_make_shared<CPUResManager>();
		m_ResGPU = safe_make_shared<GPUResManager>(m_GAPI, m_ResCPU);
		m_EntityFactory = safe_make_shared<SceneEntityFactory>(m_ResGPU);

		// Содаём основное окно(View) приложения
		m_View = safe_make_shared<View>(m_Config->GetAppWinConfig(), m_EntityFactory, m_GAPI);
		m_View->viewResized += std::bind(&Engine::OnViewResize, this, std::placeholders::_1, std::placeholders::_2);
		m_View->viewResizing += std::bind(&Engine::OnViewResizing, this);

		// Создаём главный цикл приложения
		m_MainLoop = safe_make_shared<MainLoop>();
		m_MainLoop->onUpdateSystem += std::bind(&Engine::OnUpdateSystem, this);

		// Создаём обёртки для публичного API
		m_UserView = safe_make_shared<UserView>(m_View);

		initState = eInitState::InitOK;

		return {};
	}

	Result<> Engine::Run() noexcept
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
			auto result = string_to_wstring(e.what());
			if (result)
				err = L">>>>> [Engine::Run()]. Exception: " + result.value() + L"\n";
			else
				err = L">>>>> [Engine::Run()]. Unknown exception occurred\n";
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

	// Обработка изменения размера окна в процессе изменения
	void Engine::OnViewResizing()
	{
		// TODO: в будущем настроить созранение отрендренных кадров при изменении размера окна
		// и их отрисовку для плавного зума окна без чёрных экранов
		// Сейчас просто дважды вызываем OnUpdateSystem чтобы отрисовать что-то
		OnUpdateSystem();
		OnUpdateSystem();
	}

	// Обработка изменения размера окна
	void Engine::OnViewResize(const Size2D<>& size, eTypeWinResize resizeType)
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

#pragma region User API

#pragma endregion

#pragma region Helpers
	Result<std::unique_ptr<StartupConfig>> Engine::GetStartupConfig()
	{
		ZamlParser zamlParser;
		auto res = zamlParser.GetAppWinConfig(m_ZamlStartupConfig);
		if (!res)
			Unexpected(eResult::not_initialized, res.error().getMessage());

		std::shared_ptr<AppWinConfig> winConfig = res.value();
		std::shared_ptr<GAPIConfig> gapiConfig = safe_make_shared<GAPIConfig>();

		return Result<std::unique_ptr<StartupConfig>>(safe_make_unique<StartupConfig>(winConfig, gapiConfig));
	}
#pragma endregion
}