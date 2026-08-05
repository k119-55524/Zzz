#include "Engine.h"
#include "view/ViewManager.h"
#include "platforms/mainloop/MainLoop.h"
#include "platforms/package/PackageManager.h"

using namespace zzz;
using namespace zzz::core;
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
	m_UserSettingsManager = safe_make_shared<UserSettingsManager>(*m_Path, *m_PackageManager);
	m_Platform = safe_make_unique<Platform>(nativeData);
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

			m_Scripts.clear();
			m_EventBus = nullptr;
		}

		m_Time = nullptr;
		if (m_UserSettingsManager)
			auto res = m_UserSettingsManager->SaveConfig();
		m_UserSettingsManager = nullptr;
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
		RegisterScripts();
		LoadGlobalScripts();

		auto view = m_ViewManager->InitializeFromPackage(*m_PackageManager);
		if (!view)
		{
			DOutError("{}", view.error());
			return std::unexpected(view.error());
		}

		m_EventBus->InvokeStart();
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

void Engine::RegisterScripts()
{
	RegisterAllScripts();
}

void Engine::LoadGlobalScripts()
{
	std::lock_guard lock(stateMutex);

	auto globalScripts = zzz::core::ScriptRegistry::GetAllGameScriptNames();
	for (const auto& scriptName : globalScripts)
	{
		if (auto script = zzz::core::ScriptRegistry::CreateGameScript(scriptName))
		{
			m_Scripts.push_back(script);
			script->Init(m_EventBus);
			//DOut("Глобальный скрипт инициализирован: {}", scriptName);
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
