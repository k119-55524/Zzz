
#include "engine.h"
#include "headers/enums.h"
#include "private/core/view/ViewManager.h"
#include "private/platforms/main_loop/MainLoop.h"
#include "public/core/scene/scripts/ScriptRegistry.h"
#include "public/core/scene/scripts/base_script/GameScript.h"
#include "public/core/events/EventBus.h"

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
		m_EventBus = nullptr;
		m_Time = nullptr;
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
	m_EventBus = safe_make_shared<ProjectEventBus>();
	m_Time = safe_make_shared<Time>();

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
	OnRegisterScripts();

	for (const auto& scriptName : globalScripts)
	{
		if (auto script = zzz::script::ScriptRegistry::CreateGameScript(scriptName))
		{
			m_Scripts.push_back(script);
			script->Init(m_EventBus);
			DOut("Global script initialized: {}", scriptName);
		}
		else
		{
			DOutError("Failed to create global script: {}", scriptName);
		}
	}
	
	m_EventBus->InvokeStart();
}

void Engine::StopGame()
{
	m_EventBus->InvokeStop();
	m_Scripts.clear();
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

		StartGame(zzz::script::ScriptRegistry::GetAllGameScriptNames());

		m_Time->ResetFrameTimer();
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
	m_Time->Update();
	m_EventBus->InvokeUpdate(*m_Time);

	if (m_ViewManager)
		m_ViewManager->Update(m_Time->GetTimeSinceStartup());
}
