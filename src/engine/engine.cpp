
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
	m_Platform = safe_make_unique<Platform>(appName, nativeData);
	Initialize();
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
	m_ViewManager = safe_make_unique<ViewManager>(*m_Platform, [this]() { OnCloseAllViews(); });
	m_MainLoop = safe_make_shared<MainLoop>(*m_Platform, [this]() { OnUpdateSystem(); });

	OnRegisterScripts();

	DOut("Engine initialized: OK.");
	engineState.store(eInitState::Initialized);
}

void Engine::OnRegisterScripts()
{
	// Базовая реализация пуста. Будет переопределена в редакторе или в собранной игре.
}

// Run the engine loop
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

void Engine::OnCloseAllViews() const
{
#if !Z_EDITOR
	m_MainLoop->Stop();
#endif
}

void Engine::OnUpdateSystem()
{
	zF64 currentTime = 0.0f;
	if (m_ViewManager)
		m_ViewManager->Update(currentTime);

	const int ci = 10'000'000;
	static int i = ci;
	i++;

	if (i > ci)
	{
		i = 0;
		DOut("Tick!!!");
	}
}
