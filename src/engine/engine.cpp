#include "pch.h"

#include "engine.h"
#include "private/core/IO/Path.h"
#include "private/core/Config/ConfigManager.h"

using namespace zzz;
using namespace zzz::io;
using namespace zzz::engine;

Engine::Engine(std::string_view appName, std::shared_ptr<void> platformData) :
	m_AppName{ appName },
	m_PlatformData{ platformData }
{
	engineState.store(eInitState::NotInitialized);
	ensure(m_AppName.empty() == false, "Application name must not be empty.");
}

Engine::~Engine()
{
	Shutdown();
}

void Engine::Shutdown()
{
	std::lock_guard lock(stateMutex);

	bool isSaveConfig = (engineState.load() != eInitState::NotInitialized && m_ConfigManager);
	engineState.store(eInitState::Destroying);

	try
	{
		if (isSaveConfig)
		{
			auto res = m_ConfigManager->SaveConfig();
			if (!res)
				DOutCritical("Failed to serialize config: {}.", res.error());
		}

		for (auto& view : m_NativeView)
			view = nullptr;

		m_NativeView.clear();

		m_ConfigManager = nullptr;
		m_Path = nullptr;
	}
	catch (const std::exception& e)
	{
		DOutException("Exception during shutdown: {}.", e.what());
	}
	catch (...)
	{
		DOutException("Unknown exception during shutdown.");
	}

	DOut("Engine shutdown completed.");
	engineState.store(eInitState::NotInitialized);
}

std::expected<void, std::string> Engine::Initialize(std::string_view configPath)
{
	std::lock_guard lock(stateMutex);

	if (engineState.load() != eInitState::NotInitialized)
		return UNEXPECTED("Engine is already initialized or running.");

	engineState.store(eInitState::Initializing);

	try
	{
		// Инициализация пути и менеджера конфигурации
		m_Path = zzz::safe_make_shared<Path>(m_AppName, m_PlatformData);
		m_ConfigManager = zzz::safe_make_shared<ConfigManager>(m_Path);
		auto res = m_ConfigManager->Initialize(configPath)
			.and_then([this](eInitConfigState state)
				{
					if (state == eInitConfigState::InitDefault)
						DOutWarning("Config initialized with default settings.");

					m_NativeView.push_back(zzz::safe_make_shared<NativeView>(m_AppName, m_ConfigManager->GetEngineConfig()));
					return std::expected<void, std::string>{};
				})
			.or_else([&](const std::string& error)
				-> std::expected<void, std::string>
				{
					DOutError("Initialization failed: {}", error);
					Shutdown();
					return std::unexpected(error);
				});

		DOut("Engine initialized: OK.");
		engineState.store(eInitState::Initialized);

		return {};
	}
	catch (const std::exception& e)
	{
		Shutdown();
		return UNEXPECTED("Exception initialize: {}.", e.what());
	}
	catch (...)
	{
		Shutdown();
		return UNEXPECTED("Unknown exception occurred.");
	}
}

[[nodiscard]] std::expected<void, std::string> Engine::Run()
{
	std::lock_guard lock(stateMutex);

	if (engineState.load() != eInitState::Initialized)
		return UNEXPECTED("Engine is not initialized. Call Initialize() before Run().");

	engineState.store(eInitState::Running);



	return {};
}

#pragma region Mobile Lifecycle Events
#if defined(__APPLE__)
void Engine::OnPlatformApplicationDidBecomeActive()
{
	DOut("Application did become active.");
}

void Engine::OnPlatformApplicationWillResignActive()
{
	DOut("Application will resign active.");
}

void Engine::OnPlatformApplicationDidEnterBackground()
{
	DOut("Application did enter background.");

	if (engineState.load() == eInitState::Running)
	{
		auto res = m_ConfigManager->SaveConfig();
		if (!res)
			DOutCritical("Failed to save config on entering background: {}.", res.error());
	}
}

void Engine::OnPlatformApplicationWillEnterForeground()
{
	DOut("Application will enter foreground.");
}

void Engine::OnPlatformApplicationDidReceiveMemoryWarning()
{
	DOut("Application did receive memory warning.");
}
#endif // defined(__APPLE__)

#if defined(__ANDROID__)
void Engine::OnPlatformActivityResumed()
{
	DOut("Activity resumed.");
}

void Engine::OnPlatformActivityPaused()
{
	DOut("Activity paused.");

	if (engineState.load() == eInitState::Running)
	{
		auto res = m_ConfigManager->SaveConfig();
		if (!res)
			DOutCritical("Failed to save config on entering background: {}.", res.error());
	}
}

void Engine::OnPlatformActivityStopped()
{
	DOut("Activity stopped.");
}

void Engine::OnPlatformActivityStarted()
{
	DOut("Activity started.");
}

void Engine::OnPlatformLowMemory()
{
	DOut("Low memory warning.");
}
#endif // defined(__ANDROID__)
#pragma endregion

