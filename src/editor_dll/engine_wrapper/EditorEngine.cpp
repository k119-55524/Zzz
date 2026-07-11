#include <engine/headers/enums.h>
#include <engine/private/core/view/ViewManager.h>
#include <engine/public/core/scene/scripts/ScriptRegistry.h>
#include <engine/public/core/events/EventBus.h>
#include <engine/public/core/scene/GameObject.h>
#include <engine/public/core/scene/scripts/base_script/Script.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "EditorEngine.h"

using namespace zzz;
using namespace zzz::engine;
using namespace zzz::logger;

namespace zzz::editor
{
	EditorEngine::~EditorEngine()
	{
		UnloadScripts();
	}

	[[nodiscard]] std::expected<void, std::string> EditorEngine::Run()
	{
		std::lock_guard lock(stateMutex);

		if (engineState.load() != eInitState::Initialized)
			return UNEXPECTED("Движок не инициализирован.");

		engineState.store(eInitState::Running);
		return {};
	}

	void EditorEngine::Tick()
	{
		if (engineState.load() == eInitState::Running)
		{
			std::lock_guard lock(stateMutex);
			OnUpdateSystem();
		}
	}

	void EditorEngine::ClearEngine()
	{
		UnloadScripts();
	}

	zzz::engine::View* EditorEngine::AddView(void* hwnd)
	{
		if (engineState.load() != eInitState::Running)
		{
			DOutError("EditorEngine не запущен. Невозможно выполнить AddView.");
			return nullptr;
		}

		return m_ViewManager->CreateView(hwnd);
	}

	void EditorEngine::RemoveView(void* view)
	{
		if (engineState.load() == eInitState::Running && view)
			m_ViewManager->RemoveView(static_cast<zzz::engine::View*>(view));
	}

	void EditorEngine::SetProjectPath(std::string_view path)
	{
		m_ProjectPath = path;
	}

	void EditorEngine::ReloadScripts()
	{
		UnloadScripts();

		if (m_ProjectPath.empty())
		{
			DOutError("Путь проекта пуст. Невозможно загрузить scripts DLL.");
			return;
		}

		std::string originDllPath = m_ProjectPath + "/.editor/bin/scripts.dll";
		std::string tempDllPath = m_ProjectPath + "/.editor/bin/scripts_temp_" + std::to_string(GetTickCount()) + ".dll";

		bool copied = false;
		for (int i = 0; i < 50; ++i)
		{
			if (CopyFileA(originDllPath.c_str(), tempDllPath.c_str(), FALSE))
			{
				copied = true;
				break;
			}

			DOutWarning("Новая попытка ожидания копирования DLL.");
			Sleep(100);
		}

		if (!copied)
		{
			DOutWarning("Не удалось скопировать scripts.dll в scripts_temp.dll. Возможно, DLL ещё не существует. Ошибка: {}", GetLastError());
			return;
		}

		HMODULE handle = LoadLibraryA(tempDllPath.c_str());
		if (!handle)
		{
			DOutError("Не удалось загрузить {}. Код ошибки: {}", tempDllPath, GetLastError());
			return;
		}

		m_ScriptsDll = handle;
		m_LoadedTempDllPath = tempDllPath;

		extern zzz::logger::LogCallback g_EditorLogCallback;
		using InitLogFunc = void(*)(void*);
		InitLogFunc initLogger = (InitLogFunc)GetProcAddress(handle, "InitScriptLogger");
		if (initLogger && g_EditorLogCallback)
		{
			initLogger((void*)g_EditorLogCallback);
		}

		using RegisterFunc = void(*)();
		RegisterFunc registerAll = (RegisterFunc)GetProcAddress(handle, "RegisterAllScripts");
		if (registerAll)
		{
			registerAll();
			DOut("Scripts DLL загружена и зарегистрирована успешно.");
		}
		else
		{
			DOutError("Не удалось найти экспорт RegisterAllScripts в scripts DLL.");
			UnloadScripts();
		}
	}

	void EditorEngine::UnloadScripts()
	{
		if (m_ScriptsDll)
		{
			DOut("Выгрузка scripts DLL...");

#if Z_EDITOR
			// 1. Удаляем все активные GameObject-скрипты
			auto activeScripts = zzz::script::ScriptRegistry::GetActiveInstances();
			for (auto* scriptRaw : activeScripts)
			{
				if (auto owner = scriptRaw->GetOwner())
				{
					owner->RemoveScript(std::static_pointer_cast<zzz::script::Script>(scriptRaw->shared_from_this()));
				}
			}

			// 2. Удаляем все активные GameScript-ы
			ClearGameScripts();

			// 3. (TODO Phase 3: Сцен пока нет, но тут будет удаление SceneScripts)
#endif

			zzz::script::ScriptRegistry::Clear();
			FreeLibrary((HMODULE)m_ScriptsDll);
			m_ScriptsDll = nullptr;

			if (!m_LoadedTempDllPath.empty())
			{
				DeleteFileA(m_LoadedTempDllPath.c_str());
				m_LoadedTempDllPath.clear();
			}
		}
	}

	void EditorEngine::OnUpdateSystem()
	{
		m_Time->Update();

		if (!m_IsPaused)
			m_EventBus->InvokeUpdate(*m_Time);

		if (m_ViewManager)
			m_ViewManager->Update(m_Time->GetTimeSinceStartup());
	}

	void EditorEngine::OnRegisterScripts()
	{
		if (!m_ProjectPath.empty())
		{
			ReloadScripts();
		}
	}

	void EditorEngine::Play(const char** scriptClasses, int count)
	{
		std::vector<std::string> classes;
		for (int i = 0; i < count; ++i)
		{
			if (scriptClasses[i])
			{
				classes.push_back(scriptClasses[i]);
			}
		}

		std::lock_guard lock(stateMutex);
		StartGame(classes);
		m_Time->ResetFrameTimer();
	}

	void EditorEngine::Stop()
	{
		std::lock_guard lock(stateMutex);
		StopGame();
	}

	void EditorEngine::Pause(bool isPaused)
	{
		std::lock_guard lock(stateMutex);
		m_IsPaused = isPaused;
		DOut("EditorEngine::Pause called with: {}", isPaused);
	}
}
