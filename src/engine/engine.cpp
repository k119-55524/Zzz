
#include "engine.h"
#include "headers/enums.h"
#include "private/core/view/ViewManager.h"
#include "private/platforms/main_loop/MainLoop.h"
#include "public/core/scene/scripts/ScriptRegistry.h"
#include "public/core/scene/scripts/base_script/Game.h"

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

	DOut("Engine initialized: OK.");
	engineState.store(eInitState::Initialized);
}

#if defined(_MSC_VER)
#pragma comment(linker, "/alternatename:RegisterAllScripts=DefaultRegisterAllScripts")
extern "C" void DefaultRegisterAllScripts() {}
extern "C" void RegisterAllScripts();
#else
extern "C" __attribute__((weak)) void RegisterAllScripts() {}
#endif

void Engine::OnRegisterScripts()
{
	RegisterAllScripts();
}

void Engine::StartGame(const std::vector<std::string>& globalScripts)
{
	for (const auto& scriptName : globalScripts)
	{
		if (auto game = zzz::script::ScriptRegistry::CreateGame(scriptName))
		{
			m_GlobalGames.push_back(game);
			game->OnStart();
			DOut("Global script started: {}", scriptName);
		}
		else
		{
			DOutError("Failed to create global script: {}", scriptName);
		}
	}
}

void Engine::StopGame()
{
	m_GlobalGames.clear();
	m_IsTimePaused = false;
}

void Engine::PauseGame(bool isPaused)
{
	m_IsTimePaused = isPaused;
}

[[nodiscard]] std::expected<void, std::string> Engine::Run()
{
	{
		std::lock_guard lock(stateMutex);

		if (engineState.load() != eInitState::Initialized)
			return UNEXPECTED("Engine is not initialized. Call Initialize() before Run().");

		engineState.store(eInitState::Running);
	}

	std::string err;
	bool isError = false;
	try
	{
		m_ViewManager->CreateView();
		m_ViewManager->CreateView();

		OnRegisterScripts();
		StartGame(zzz::script::ScriptRegistry::GetAllGameNames());

		m_MainLoop->Run();

		if constexpr (!Platform::c_AsyncRunLoop)
			StopGame();
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
	zF64 currentTime = 0.0f; // TODO: Calculate actual delta time
	if (m_ViewManager)
		m_ViewManager->Update(currentTime);

	if (!m_IsTimePaused)
	{
		for (auto& game : m_GlobalGames)
		{
			game->OnUpdate(static_cast<float>(currentTime));
		}
	}

	const int ci = 10'000'000;
	static int i = ci;
	i++;

	if (i > ci)
	{
		i = 0;
		// DOut("Tick!!!");
	}
}
