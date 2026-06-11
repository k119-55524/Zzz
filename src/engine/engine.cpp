
#include "engine.h"
#include "headers/enums.h"
#include "private/core/view/ViewManager.h"
#include "private/platforms/main_loop/MainLoop.h"

using namespace zzz;
using namespace zzz::io;
using namespace zzz::engine;

Engine::Engine(std::string_view appName, std::shared_ptr<NativeAppData> nativeData) :
	engineState{ eInitState::NotInitialized }
{
	m_Platform = safe_make_shared<Platform>(appName, nativeData);
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
		m_ViewManager = nullptr;
		m_MainLoop = nullptr;
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
		m_MainLoop = safe_make_shared<MainLoop>(m_Platform, std::bind(&Engine::OnUpdateSystem, this));
		m_ViewManager = safe_make_unique<ViewManager>(m_Platform, std::bind(&Engine::OnCloseAllViews, this));
		m_ViewManager->CreateView();
		//m_ViewManager->CreateView();

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

#if !defined(Z_APPLE)
	Shutdown();
#endif

	if (isError)
	{
		DOutException("Exception during Run: {}.", err);
		//MsgBox::Error(err);
		return UNEXPECTED("Exception during Run: {}.", err);
	}

	return {};
}

void Engine::OnCloseAllViews()
{
	m_MainLoop->Stop();
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