#include "pch.h"

#include "engine.h"
#include "private/core/IO/Path.h"
#include "private/core/Config/ConfigManager.h"

using namespace zzz;
using namespace zzz::io;
using namespace zzz::engine;

Engine::Engine(std::string_view appName, std::shared_ptr<void> platformData) :
	m_AppName{ appName },
	m_PlatformData{ platformData },
	initState{ eInitState::NotInitialized }
{
	ensure(m_AppName.empty() == false, "Application name must not be empty.");
}

Engine::~Engine()
{
	Shutdown();
}

void Engine::Shutdown()
{
	try
	{
		if (initState != eInitState::NotInitialized)
		{
			auto res = m_ConfigManager->SaveConfig();
			if (!res)
				DOutCritical("Failed to serialize config: {}.", res.error());
		}

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
	initState.store(eInitState::NotInitialized);
}

std::expected<void, std::string> Engine::Initialize(std::string_view configPath)
{
	std::lock_guard lock(stateMutex);

	if (initState != eInitState::NotInitialized)
		UNEXPECTED("Engine is already initialized or running.");

	try
	{
		DOut("Engine initialized: START.");

		m_Path = zzz::safe_make_shared<Path>(m_AppName, m_PlatformData);
		m_ConfigManager = zzz::safe_make_shared<ConfigManager>(m_Path);
		auto res = m_ConfigManager->Initialize(configPath);
		if (!res)
			UNEXPECTED("Failed to initialize ConfigManager: {}.", res.error());



		DOut("Engine initialized: END.");
		initState.store(eInitState::Initialized);

		return {};
	}
	catch (const std::exception& e)
	{
		UNEXPECTED("Exception initialize: {}.", e.what());
	}
	catch (...)
	{
		UNEXPECTED("Unknown exception occurred.");
	}
}

[[nodiscard]] std::expected<void, std::string> Engine::Run()
{
	std::lock_guard lock(stateMutex);

	if (initState != eInitState::Initialized)
		UNEXPECTED("Engine is not initialized. Call Initialize() before Run().");

	initState.store(eInitState::Running);



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

	if (initState.load() == eInitState::Running)
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