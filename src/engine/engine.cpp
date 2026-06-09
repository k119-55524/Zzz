#include "pch.h"

#include <foundation.h>

#include "engine.h"
#include "headers/enums.h"
#include "private/platforms/native_view/NativeView.h"

#include "private/factories/EngineFactory.h"

using namespace zzz;
using namespace zzz::io;
using namespace zzz::engine;

Engine::Engine(std::string_view appName, std::shared_ptr<PlatformNativeData> platformData) :
	engineState{ eInitState::NotInitialized }
{
	m_Platform = zzz::safe_make_shared<Platform>(appName, platformData);
	m_Platform->Initialize();
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
		m_MainLoop = m_Platform->GetFactory()->CreateMainLoop(m_Platform);
		m_MainLoop->onUpdateSystem += std::bind(&Engine::OnUpdateSystem, this);

		AddView();
		//AddView();

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

void Engine::AddView()
{
#if defined(Z_MOBILE)
	if (m_NativeViews.size() >= 1)
		THROW_RUNTIME("Mobile platforms support only one native window per application.");
#endif

	auto view = zzz::safe_make_shared<NativeView>(m_Platform);

	// Добавляем слушателя на закрытие окна
	view->GetWindow()->onCloseRequested += [this, weakView = std::weak_ptr(view)]()
	{
		if (auto v = weakView.lock())
			m_NativeViews.remove(v);

		// Закрываем приложение в отсуутствии активных окон
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
#if defined(Z_APPLE)
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
#endif // defined(Z_APPLE)
#pragma endregion
