
#include <engine/headers/enums.h>
#include <engine/private/core/view/ViewManager.h>
#include <engine/private/core/scene/scripts/ScriptRegistry.h>

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
			return UNEXPECTED("Engine is not initialized.");

		engineState.store(eInitState::Running);
		return {};
	}

	void EditorEngine::Tick()
	{
		if (engineState.load() == eInitState::Running)
		{
			OnUpdateSystem();
		}
	}

	void EditorEngine::ClearEngine()
	{
		
	}

	zzz::engine::View* EditorEngine::AddView(void* hwnd)
	{
		if (engineState.load() != eInitState::Running)
		{
			DOutError("EditorEngine is not running. Cannot AddView.");
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
			DOutError("EditorEngine: Project path is empty. Cannot load scripts DLL.");
			return;
		}

		std::string originDllPath = m_ProjectPath + "/bin/scripts.dll";
		std::string tempDllPath = m_ProjectPath + "/bin/scripts_temp.dll";

		// Копируем во временный файл, чтобы не лочить оригинальный DLL для компиляции
		if (!CopyFileA(originDllPath.c_str(), tempDllPath.c_str(), FALSE))
		{
			DOutWarning("EditorEngine: Failed to copy scripts.dll to scripts_temp.dll. DLL might not exist yet.");
			return;
		}

		HMODULE handle = LoadLibraryA(tempDllPath.c_str());
		if (!handle)
		{
			DOutError("EditorEngine: Failed to load scripts_temp.dll. Error code: {}", GetLastError());
			return;
		}

		m_ScriptsDll = handle;

		using RegisterFunc = void(*)();
		RegisterFunc registerAll = (RegisterFunc)GetProcAddress(handle, "RegisterAllScripts");
		if (registerAll)
		{
			registerAll();
			DOut("EditorEngine: Scripts DLL loaded and registered successfully.");
		}
		else
		{
			DOutError("EditorEngine: Failed to find RegisterAllScripts export in scripts DLL.");
			UnloadScripts();
		}
	}

	void EditorEngine::UnloadScripts()
	{
		if (m_ScriptsDll)
		{
			DOut("EditorEngine: Unloading scripts DLL...");

			// Очищаем зарегистрированные фабрики
			zzz::script::ScriptRegistry::Clear();

			FreeLibrary((HMODULE)m_ScriptsDll);
			m_ScriptsDll = nullptr;

			std::string tempDllPath = m_ProjectPath + "/bin/scripts_temp.dll";
			DeleteFileA(tempDllPath.c_str());
		}
	}

	void EditorEngine::OnRegisterScripts()
	{
		if (!m_ProjectPath.empty())
		{
			ReloadScripts();
		}
	}
}
