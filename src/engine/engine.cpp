
#include <logger/logger.h>

#include "Engine.h"

Z_SET_LOG_CATEGORY(::zzz::core::LogEngine);

using namespace zzz;
using namespace zzz::core;
using namespace zzz::engine;
using namespace zzz::logger;

#if defined(_MSC_VER)
#pragma comment(linker, "/alternatename:RegisterAllScripts=DefaultRegisterAllScripts")
extern "C" void DefaultRegisterAllScripts(zzz::core::ScriptRegistry&) {}
extern "C" void RegisterAllScripts(zzz::core::ScriptRegistry&);
#else
extern "C" __attribute__((weak)) void RegisterAllScripts(zzz::core::ScriptRegistry&) {}
#endif

Engine::Engine(std::shared_ptr<NativeAppData> nativeData) :
	engineState{ eInitState::NotInitialized }
{
	m_Path = safe_make_shared<Path>(nativeData);
	m_PackageManager = safe_make_shared<PackageManager>(*m_Path);
	if (auto res = m_Path->InitializeUserData(m_PackageManager->GetCompanyName(), m_PackageManager->GetAppName()); !res)
		THROW_RUNTIME("Не удалось инициализировать каталог пользовательских данных: {}", res.error());

	auto projectManifestData = m_PackageManager->GetProjectManifestData();
	if (!projectManifestData)
		THROW_RUNTIME("Failed to load ProjectManifestData: {}", projectManifestData.error());

	// Установка максимального размера сетевой очереди логов из манифеста
	g_Logger.SetMaxNetworkLogQueueSize(projectManifestData->GetMaxLogQueueSize());

	auto primaryViewData = m_PackageManager->GetPrimaryViewData();
	if (!primaryViewData)
		THROW_RUNTIME("Failed to load PrimaryViewData: {}", primaryViewData.error());

	// Загрузка пользовательских настроек (UserSettings.dat)
	m_UserSettingsManager = safe_make_shared<UserSettingsManager>(*m_Path);

	// Создание платформенного слоя абстракции ОС (native windows, ввод, системные события)
	m_Platform = safe_make_unique<Platform>(nativeData, projectManifestData->GetPlatformData());
	m_Platform->GetHardwareState().LogFileBlock();

	// Инициализация графического интерфейса (DirectX 12 / Vulkan / Metal)
	m_GAPI = safe_make_shared<GAPI>();
	m_GAPI->Initialize(m_UserSettingsManager);

	// Инициализация изолированной подсистемы скриптов (хранилище, регистратор и фабрика экземпляра движка)
	m_ScriptStorage = safe_make_shared<ScriptStorage>();
	m_ScriptRegistry = safe_make_unique<ScriptRegistry>(*m_ScriptStorage);
	m_ScriptFactory = safe_make_shared<ScriptFactory>(*m_ScriptStorage);

	// Инициализация менеджера сцен (SceneManager) - используется View для загрузки стартовых сцен
	m_SceneManager = safe_make_shared<SceneManager>(m_PackageManager, m_ScriptFactory);

	// Инициализация менеджера отображения окон (ViewManager) с пробросом графического API, фабрики скриптов, пакета ресурсов и менеджера сцен
	m_ViewManager = safe_make_unique<ViewManager>(*m_Platform, m_GAPI, m_ScriptFactory, m_PackageManager, m_UserSettingsManager, m_SceneManager, [this]() { OnAppClosed(); });

	// Инициализация главного кадрового цикла, шины событий проекта и игрового таймера
	m_MainLoop = safe_make_shared<MainLoop>(*m_Platform, [this]() { OnUpdateSystem(); });
	m_EventBus = safe_make_shared<ProjectEventBus>();

	// Сохраняем пользовательскую конфигурацию на диск, если в процессе инициализации были изменения
	if (auto res = m_UserSettingsManager->SaveConfig(); !res)
		DOutWarning("[Engine::Engine] Не удалось сохранить пользовательские настройки после инициализации: {}", res.error());

	engineState.store(eInitState::Initialized);

	DOut("[Engine::Engine] - Инициализация: OK.");
}

Engine::~Engine()
{
	Shutdown();
}

void Engine::Shutdown()
{
	engineState.store(eInitState::Destroying);

	try
	{
		m_MainLoop = nullptr;
		m_ViewManager = nullptr;
		m_GAPI = nullptr;

		{
			if (m_EventBus)
				m_EventBus->InvokeDestroy();

			m_Scripts.clear();
			m_EventBus = nullptr;
		}

		m_Time = nullptr;

		if (m_UserSettingsManager)
			auto res = m_UserSettingsManager->SaveConfig();
		m_UserSettingsManager = nullptr;

		m_SceneManager = nullptr;
		m_Platform = nullptr;
		m_PackageManager = nullptr;
		m_Path = nullptr;
	}
	catch (const std::exception& e)
	{
		DOutException("Исключение при завершении работы: {}.", e.what());
	}

	engineState.store(eInitState::NotInitialized);
}

[[nodiscard]] std::expected<void, std::string> Engine::Run()
{
	{
		std::lock_guard lock(stateMutex);

		if (engineState.load() != eInitState::Initialized)
			return UNEXPECTED("Движок не инициализирован. Вызовите Initialize() перед Run().");

		engineState.store(eInitState::Running);
	}

	std::string err;
	bool isError = false;
	try
	{
		RegisterScripts();
		LoadGlobalScripts();

		m_ViewManager->CreatePrimaryView();

		// Восстановление состояния Child/Independent окон из user.dat, если они были открыты в прошлой сессии
#if !Z_MOBILE
		// Тест: принудительное создание ВСЕХ Child/Independent окон, какие есть
		//for (const auto& guid : m_PackageManager->GetAllGuidsOfType(ePackage::ChildView))
		//	m_ViewManager->CreateChildView(guid);
		//for (const auto& guid : m_PackageManager->GetAllGuidsOfType(ePackage::IndependentView))
		//	m_ViewManager->CreateIndependentView(guid);

		std::vector<Guid> staleChildGuids;
		for (const auto& [guid, userData] : m_UserSettingsManager->GetChildViewsUserData())
		{
			if (userData.GetPlatformData().GetWindowState() == eWindowState::Closed)
				continue;

			if (!m_PackageManager->HasEntry(ePackage::ChildView, guid))
			{
				staleChildGuids.push_back(guid);
				continue;
			}

			m_ViewManager->CreateChildView(guid);
		}
		for (const auto& guid : staleChildGuids)
			m_UserSettingsManager->RemoveChildViewUserData(guid);

		std::vector<Guid> staleIndependentGuids;
		for (const auto& [guid, userData] : m_UserSettingsManager->GetIndependentViewsUserData())
		{
			if (userData.GetPlatformData().GetWindowState() == eWindowState::Closed)
				continue;

			if (!m_PackageManager->HasEntry(ePackage::IndependentView, guid))
			{
				staleIndependentGuids.push_back(guid);
				continue;
			}

			m_ViewManager->CreateIndependentView(guid);
		}
		for (const auto& guid : staleIndependentGuids)
			m_UserSettingsManager->RemoveIndependentViewUserData(guid);
#endif

		m_EventBus->InvokeStart();
		m_Time = safe_make_shared<Time>();
		m_MainLoop->Run();
	}
	catch (const std::exception& e)
	{
		isError = true;
		err = e.what();
	}
	catch (...)
	{
		isError = true;
		err = "Произошло неизвестное исключение";
	}

	if constexpr (!Platform::c_AsyncRunLoop)
		Shutdown();

	if (isError)
	{
		DOutException("Исключение во время Run: {}.", err);
		//MsgBox::Error(err);
		return UNEXPECTED("Исключение во время Run: {}.", err);
	}

	return {};
}

void Engine::RegisterScripts()
{
	RegisterAllScripts(*m_ScriptRegistry);
}

void Engine::LoadGlobalScripts()
{
	std::lock_guard lock(stateMutex);

	auto globalScripts = m_ScriptFactory->GetAllGameScriptNames();
	for (const auto& scriptName : globalScripts)
	{
		if (auto script = m_ScriptFactory->CreateGameScript(scriptName))
		{
			m_Scripts.push_back(script);
			script->Init(m_EventBus);
			//DOut("Глобальный скрипт инициализирован: {}", scriptName);
		}
		else
			DOutError("Не удалось создать глобальный скрипт: {}", scriptName);
	}
}

void Engine::OnAppClosed() const
{
#if !Z_EDITOR
	m_MainLoop->Stop();
#endif
}

void Engine::OnUpdateSystem()
{
	m_Time->Update();

	// Вывод среднего FPS в лог каждые logInterval секунд
#if Z_ADD_LOGGER
	{
		// Интервал (в секундах) для расчёта среднего FPS и вывода в лог.
		// Измените logInterval (например, 1.0f, 2.0f, 5.0f), чтобы изменить частоту вывода и период усреднения.
		constexpr float logInterval = 5.0f;

		static float accumTime = 0.0f;
		static uint32_t frameCount = 0;

		accumTime += m_Time->GetUnscaledDeltaTime();
		frameCount++;

		// Вывод лога и сброс аккумуляторов происходит по истечении интервала задержки
		if (accumTime >= logInterval)
		{
			const float avgFps = static_cast<float>(frameCount) / accumTime;
			const float avgFrameTimeMs = (accumTime / static_cast<float>(frameCount)) * 1000.0f;

			DOut("[Engine::OnUpdateSystem] FPS (Avg {:.0f}s): {:.1f} (FrameTime: {:.2f}ms)", logInterval, avgFps, avgFrameTimeMs);

			accumTime = 0.0f;
			frameCount = 0;
		}
	}
#endif // Z_ADD_LOGGER

	m_EventBus->InvokeUpdate(*m_Time);

	if (m_SceneManager)
		m_SceneManager->Update(*m_Time);

	if (m_ViewManager)
		m_ViewManager->Update(*m_Time);
}
