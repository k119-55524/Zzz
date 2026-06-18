
#include "engine.h"
#include "headers/enums.h"
#include "private/core/view/ViewManager.h"
#include "private/platforms/main_loop/MainLoop.h"

using namespace zzz;
using namespace zzz::common;
using namespace zzz::engine;

Engine::Engine(std::string_view appName, std::shared_ptr<NativeAppData> nativeData) :
	engineState{ eInitState::NotInitialized }
{
	ensure(s_Instance == nullptr, "Engine instance already exists!");
	s_Instance = this;

	m_Platform = safe_make_unique<Platform>(appName, nativeData);
}

Engine::~Engine()
{
	Shutdown();
	s_Instance = nullptr;
}

Engine& Engine::Get()
{
	ensure(s_Instance != nullptr, "Engine is not initialized!");
	return *s_Instance;
}

void Engine::Shutdown()
{
	engineState.store(eInitState::Destroying);

	try
	{
		m_MainLoop = nullptr;
		m_ViewManager = nullptr;
		m_Platform = nullptr;
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
		m_ViewManager = safe_make_unique<ViewManager>(*m_Platform, std::bind(&Engine::OnCloseAllViews, this));

#if !Z_EDITOR
		m_MainLoop = safe_make_shared<MainLoop>(*m_Platform, std::bind(&Engine::OnUpdateSystem, this));
		m_ViewManager->CreateView();
		m_ViewManager->CreateView();
#endif

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

#if Z_EDITOR
	try
	{
		m_ViewManager->CreateView();
	}
	catch (const std::exception& e)
	{
		Shutdown();
		return UNEXPECTED("Exception creating view in Editor Run: {}.", e.what());
	}
	catch (...)
	{
		Shutdown();
		return UNEXPECTED("Unknown exception creating view in Editor Run.");
	}

	return {};
#else
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

	if constexpr (!Platform::c_AsyncRunLoop)
		Shutdown();

	if (isError)
	{
		DOutException("Exception during Run: {}.", err);
		//MsgBox::Error(err);
		return UNEXPECTED("Exception during Run: {}.", err);
	}

	return {};
#endif
}

void Engine::OnCloseAllViews()
{
#if !Z_EDITOR
	m_MainLoop->Stop();
#endif
}

void Engine::OnUpdateSystem()
{
	const int ci = 10'000'000;
	static int i = ci;
	i++;

	if (i > ci)
	{
		i = 0;
		DOut("Tick!!!");
	}
}

#if Z_EDITOR
void Engine::Tick()
{
	if (engineState.load() == eInitState::Running)
	{
		OnUpdateSystem();
	}
}
#endif