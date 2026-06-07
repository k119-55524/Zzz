#include "pch.h"

#include <foundation.h>

#include "engine.h"
#include "headers/enums.h"
#include "private/platforms/native_view/NativeView.h"

#pragma region Platform-specific includes and typedefs
#include "private/platforms/main_loop/MainLoop_MSWin.h"
#include "private/platforms/platforms/PlatformMSWindows.h"
#include "private/platforms/platforms/PlatformLinux.h"
#include "private/platforms/main_loop/MainLoop_Linux.h"

namespace zzz::engine
{
#if defined(Z_WINDOWS)
	typedef zzz::engine::PlatformMSWindows Platform;
	typedef zzz::engine::MainLoop_MSWin MainLoop;
#elif defined(Z_LINUX)
	typedef zzz::engine::PlatformLinux Platform;
	typedef zzz::engine::MainLoop_Linux MainLoop;
#else
#error ">>>>> [Compile error]. This branch requires implementation for the current platform"
#endif
}
#pragma endregion

using namespace zzz;
using namespace zzz::io;
using namespace zzz::engine;

Engine::Engine(std::string_view appName, std::string_view configPath, std::shared_ptr<void> platformData) :
	engineState{ eInitState::NotInitialized }
{
	m_Platform = safe_make_shared<Platform>(appName, platformData);
	m_Platform->Initialize(configPath);
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
		m_Platform = nullptr;
		m_MainLoop = nullptr;

		for (auto& view : m_NativeViews)
			view = nullptr;

		m_NativeViews.clear();
	}
	catch (const std::exception& e)
	{
		DOutException("Exception during shutdown: {}.", e.what());
	}
	catch (...)
	{
		DOutException("Unknown exception during shutdown.");
	}

	engineState.store(eInitState::NotInitialized);
}

std::expected<void, std::string> Engine::Initialize()
{
	std::lock_guard lock(stateMutex);

	if (engineState.load() != eInitState::NotInitialized)
		return UNEXPECTED("Engine is already initialized or running.");

	engineState.store(eInitState::Initializing);

	try
	{
		m_MainLoop = safe_make_shared<MainLoop>(m_Platform);
		m_MainLoop->onUpdateSystem += std::bind(&Engine::OnUpdateSystem, this);

		AddNativeView();
		AddNativeView();

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
	std::string err;
	bool isError = false;
	try
	{
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
		err = "Unknown exception occurred";
	}

	Shutdown();

	if (isError)
	{
		DOutException("Exception during Run: {}.", err);
		//MsgBox::Error(err);
		return UNEXPECTED("Exception during Run: {}.", err);
	}

	return {};
}

void Engine::AddNativeView()
{
	auto view = zzz::safe_make_shared<NativeView>(m_Platform);

	// Обрабатываем закрытие очередного окна
	view->GetWindow()->onCloseRequested += [this, weakView = std::weak_ptr(view)]()
	{
		if (auto v = weakView.lock())
			m_NativeViews.remove(v);

		if (m_NativeViews.empty())
			m_MainLoop->Stop();
	};

	m_NativeViews.push_back(std::move(view));
}

void Engine::OnUpdateSystem()
{
	static int i = 0;
	i++;

	if (i == 1'000'000)
	{
		i = 0;
		DOut("Tick!!!");
	}


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

