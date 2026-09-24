
#include "core/time/Time.h"
#include "core/logger/logger.h"
#include "core/events/EventBus.h"
#include "engine/view/ViewManager.h"
#include "core/utils/MemoryUtils.h"
#include "engine/platforms/Platform.h"
#include "engine/scene/SceneManager.h"
#include "engine/tasks/TaskDispatcher.h"
#include "engine/package/PackageManager.h"
#include "core/userscripts/ScriptFactory.h"
#include "core/userscripts/ScriptStorage.h"
#include "core/userscripts/ScriptRegistry.h"
#include "core/constants/PackagesConstants.h"
#include "core/io/package/DataAssetsManager.h"
#include "engine/package/UserSettingsManager.h"
#include "engine/platforms/mainloop/MainLoop.h"
#include "core/userscripts/base_script/GameScript.h"
#include "engine/resources/cpu/CpuResourceManager.h"
#include "engine/resources/gpu/GpuResourceManager.h"

#include "Engine.h"

Z_SET_LOG_CATEGORY(::zzz::core::LogEngine);

using namespace zzz;
using namespace zzz::core;
using namespace zzz::engine;
using namespace zzz::logger;

#if defined(_MSC_VER)
#pragma comment(linker, "/alternatename:RegisterAllScripts=DefaultRegisterAllScripts")
extern "C" void DefaultRegisterAllScripts(ScriptRegistry&) {}
extern "C" void RegisterAllScripts(ScriptRegistry&);
#else
extern "C" __attribute__((weak)) void RegisterAllScripts(ScriptRegistry&) {}
#endif

Engine::Engine(std::shared_ptr<NativeAppData> nativeData) :
	engineState{ eInitState::NotInitialized }
{
	m_FileSystem = safe_make_shared<FileSystem>(nativeData);

	auto pkgPath = m_FileSystem->GetGamePackagePath();
	if (!pkgPath)
		THROW_RUNTIME("Не удалось определить путь к главному пакету: {}", pkgPath.error());

	auto dataPath = m_FileSystem->GetDataPackagePath();
	if (!dataPath)
		THROW_RUNTIME("Не удалось определить путь к пакету данных: {}", dataPath.error());

	m_PackageManager = safe_make_shared<PackageManager>(*pkgPath);
	m_DataAssetsManager = safe_make_shared<DataAssetsManager>(*dataPath);
	auto userConfigPath = m_FileSystem->GetUserConfigPath(m_PackageManager->GetCompanyName(), m_PackageManager->GetAppName());
	if (!userConfigPath)
		THROW_RUNTIME("Не удалось получить путь к файлу настроек пользователя: {}", userConfigPath.error());

	// Установка максимального размера сетевой очереди логов из манифеста
	const auto& projectManifestData = m_PackageManager->GetProjectManifestData();
	g_Logger.SetMaxNetworkLogQueueSize(projectManifestData.GetMaxLogQueueSize());

	// Загрузка пользовательских настроек и валидация по пакетам ресурсов
	m_UserSettingsManager = safe_make_shared<UserSettingsManager>(*userConfigPath);
	m_UserSettingsManager->ValidateAgainstPackages(*m_PackageManager, m_DataAssetsManager.get());

	// Создание платформенного слоя абстракции ОС
	m_Platform = safe_make_unique<Platform>(nativeData, projectManifestData.GetPlatformData());
	m_Platform->GetHardwareState().LogFileBlock();

	// Инициализация централизованного диспетчера задач
	m_TaskDispatcher = safe_make_unique<TaskDispatcher>(m_Platform->GetHardwareState().GetCpuTopology());

	// Инициализация графического интерфейса
	m_GAPI = safe_make_shared<GAPI>();
	m_GAPI->Initialize(m_UserSettingsManager);

	// Инициализация центрального менеджера ресурсов CPU
	m_CpuResourceManager = safe_make_shared<CpuResourceManager>(*m_TaskDispatcher, m_DataAssetsManager);

	// Инициализация менеджера ресурсов GPU
	m_GpuResourceManager = safe_make_shared<GpuResourceManager>(m_GAPI, m_CpuResourceManager);

	// Инициализация изолированной подсистемы скриптов
	m_ScriptStorage = safe_make_shared<ScriptStorage>();
	m_ScriptRegistry = safe_make_unique<ScriptRegistry>(*m_ScriptStorage);
	m_ScriptFactory = safe_make_shared<ScriptFactory>(*m_ScriptStorage);

	// Инициализация менеджера сцен
	m_SceneManager = safe_make_shared<SceneManager>(*m_TaskDispatcher, m_PackageManager, m_CpuResourceManager, m_GpuResourceManager, m_ScriptFactory);

	// Инициализация менеджера отображения окон
	m_ViewManager = safe_make_unique<ViewManager>(*m_TaskDispatcher, *m_Platform, m_GAPI, m_ScriptFactory, m_PackageManager, m_UserSettingsManager, m_SceneManager, [this]() { OnAppClosed(); });

	// Инициализация главного кадрового цикла, шины событий проекта и игрового таймера
	m_MainLoop = safe_make_shared<MainLoop>(*m_Platform, [this]() { OnUpdateSystem(); });
	m_EventBus = safe_make_shared<ProjectEventBus>();

	// Сохраняем пользовательскую конфигурацию на диск
	if (auto res = m_UserSettingsManager->SaveConfig(); !res)
		DOutWarning("[Engine::Engine] Не удалось сохранить пользовательские настройки после инициализации: {}", res.error());

	engineState.store(eInitState::Initialized);

	DOut("[Engine::Engine] - Инициализация: OK.");
}

Engine::~Engine()
{
	Shutdown();
}

void Engine::StopGame()
{
	if (m_EventBus)
	{
		m_EventBus->InvokeDestroy();
		m_EventBus->ClearAll();
	}

	m_Scripts.clear();
}

void Engine::Shutdown() noexcept
{
	{
		std::lock_guard lock(stateMutex);
		const auto currentState = engineState.load();
		if (currentState == eInitState::NotInitialized || currentState == eInitState::Destroying)
			return;

		engineState.store(eInitState::Destroying);
	}

	try
	{
		if (m_MainLoop)
		{
			m_MainLoop->Stop();
			m_MainLoop = nullptr;
		}

		StopGame();
		m_EventBus = nullptr;

		if (m_TaskDispatcher)
			m_TaskDispatcher->JoinAll();

		if (m_GAPI)
			m_GAPI->WaitForGpu();

		m_SceneManager = nullptr;
		m_ViewManager = nullptr;
		m_CpuResourceManager = nullptr;
		m_GpuResourceManager = nullptr;

		m_GAPI = nullptr;
		m_Time = nullptr;
		m_TaskDispatcher = nullptr;

		if (m_UserSettingsManager)
		{
			if (auto res = m_UserSettingsManager->SaveConfig(); !res)
				DOutWarning("Не удалось сохранить '{}': {}", c_UserConfigFileName, res.error());
			m_UserSettingsManager = nullptr;
		}

		m_ScriptFactory = nullptr;
		m_ScriptRegistry = nullptr;
		m_ScriptStorage = nullptr;

		m_Platform = nullptr;
		m_DataAssetsManager = nullptr;
		m_PackageManager = nullptr;
		m_FileSystem = nullptr;
	}
	catch (const std::exception& e)
	{
		DOutException("[Engine::Shutdown] Исключение при завершении работы: {}.", e.what());
	}
	catch (...)
	{
		DOutException("[Engine::Shutdown] Неизвестное исключение при завершении работы.");
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

		// Восстановление состояния Child/Independent окон из пользовательской конфигурации, если они были открыты в прошлой сессии
#if !Z_MOBILE
		for (const auto& [guid, userData] : m_UserSettingsManager->GetChildViewsUserData())
		{
			if (userData.GetPlatformData().GetWindowState() == eWindowState::Closed)
				continue;

			m_ViewManager->CreateChildView(guid);
		}

		for (const auto& [guid, userData] : m_UserSettingsManager->GetIndependentViewsUserData())
		{
			if (userData.GetPlatformData().GetWindowState() == eWindowState::Closed)
				continue;

			m_ViewManager->CreateIndependentView(guid);
		}
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
		DOutException("{}", err);
		//MsgBox::Error(err);
		return std::unexpected(err);
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
	m_SceneManager->Update(*m_Time);
	m_ViewManager->Update(*m_Time);
}
