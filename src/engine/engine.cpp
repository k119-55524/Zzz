
#include "engine.h"
#include "headers/enums.h"
#include "public/core/events/EventBus.h"
#include "private/core/view/ViewManager.h"
#include "private/platforms/main_loop/MainLoop.h"
#include "private/platforms/package/PackageManager.h"
#include "public/core/userscripts/ScriptRegistry.h"
#include "public/core/userscripts/base_script/GameScript.h"

using namespace zzz;
using namespace zzz::common;
using namespace zzz::engine;

#if defined(_MSC_VER)
#pragma comment(linker, "/alternatename:RegisterAllScripts=DefaultRegisterAllScripts")
extern "C" void DefaultRegisterAllScripts() {}
extern "C" void RegisterAllScripts();
#else
extern "C" __attribute__((weak)) void RegisterAllScripts() {}
#endif

Engine::Engine(std::string_view appName, std::shared_ptr<NativeAppData> nativeData) :
	engineState{ eInitState::NotInitialized }
{
	ensure(appName.empty() == false, "Имя приложения не должно быть пустым.");

	m_Path = safe_make_shared<Path>(appName, nativeData);
	m_PackageManager = safe_make_shared<PackageManager>(*m_Path);
	m_Platform = safe_make_unique<Platform>(*m_Path, nativeData);
	m_ViewManager = safe_make_unique<ViewManager>(*m_Platform, [this]() { OnCloseAllViews(); });
	m_MainLoop = safe_make_shared<MainLoop>(*m_Platform, [this]() { OnUpdateSystem(); });
	m_EventBus = safe_make_shared<ProjectEventBus>();
	m_Time = safe_make_shared<Time>();

	engineState.store(eInitState::Initialized);
	DOut("Инициализация: OK.");
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

		{
			if (m_EventBus)
				m_EventBus->InvokeDestroy();

			for (const auto& script : m_Scripts)
				if (script)
					script->SetActive(false);

			m_Scripts.clear();
			m_EventBus = nullptr;
		}

		m_Time = nullptr;
		m_Platform = nullptr;
		m_PackageManager = nullptr;
		m_Path = nullptr;
	}
	catch (const std::exception& e)
	{
		DOutException("Исключение при завершении работы: {}.", e.what());
	}

	engineState.store(eInitState::NotInitialized);
}

[[nodiscard]] std::expected<void, std::string> Engine::Run()
{
	{
		std::lock_guard lock(stateMutex);

		if (engineState.load() != eInitState::Initialized)
			return UNEXPECTED("Движок не инициализирован. Вызовите Initialize() перед Run().");

		engineState.store(eInitState::Running);
	}

	std::string err;
	bool isError = false;
	try
	{
		m_ViewManager->CreateView("Main View", {});

		StartGame();
		m_Time->ResetFrameTimer();
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
		err = "Произошло неизвестное исключение";
	}

	if constexpr (!Platform::c_AsyncRunLoop)
		Shutdown();

	if (isError)
	{
		DOutException("Исключение во время Run: {}.", err);
		//MsgBox::Error(err);
		return UNEXPECTED("Исключение во время Run: {}.", err);
	}

	return {};
}

void Engine::StartGame()
{
	RegisterScripts();
	LoadGlobalScripts();
	m_EventBus->InvokeStart();
}

void Engine::RegisterScripts()
{
	RegisterAllScripts();
}

void Engine::LoadGlobalScripts()
{
	std::lock_guard lock(stateMutex);

	auto globalScripts = zzz::script::ScriptRegistry::GetAllGameScriptNames();
	for (const auto& scriptName : globalScripts)
	{
		if (auto script = zzz::script::ScriptRegistry::CreateGameScript(scriptName))
		{
			m_Scripts.push_back(script);
			script->Init(m_EventBus);
			DOut("Глобальный скрипт инициализирован: {}", scriptName);
		}
		else
			DOutError("Не удалось создать глобальный скрипт: {}", scriptName);
	}
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
		m_ViewManager->Update(*m_Time);
}
