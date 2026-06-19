
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
	Initialize();
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

void Engine::Initialize()
{
	m_ViewManager = safe_make_unique<ViewManager>(*m_Platform, std::bind(&Engine::OnCloseAllViews, this));
	m_MainLoop = safe_make_shared<MainLoop>(*m_Platform, std::bind(&Engine::OnUpdateSystem, this));

	DOut("Engine initialized: OK.");
	engineState.store(eInitState::Initialized);
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
		m_ViewManager->CreateView();
		m_ViewManager->CreateView();

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
