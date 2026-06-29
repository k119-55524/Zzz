
#define Z_PRINT_DEFINES
#ifdef Z_PRINT_DEFINES
#endif
#include <memory>
#include <common/common.h>
#include <engine/engine.h>
#include "../EngineWrapper/EditorEngine.h"

#include "editorDLL.h"

std::unique_ptr<zzz::editor::EditorEngine> g_Engine;

extern "C"
{
	EDITOR_API bool Initialize(zzz::logger::LogCallback callback)
	{
		try
		{
#if Z_ADD_LOGGER || Z_DEVELOPMENT_BUILD
			zzz::logger::g_Logger.AddCallbackBroadcaster(callback);
#endif

			g_Engine = zzz::safe_make_unique<zzz::editor::EditorEngine>("ZzzEditorWin");
			DOut("EditorDLL initialized: Successed.");

			auto runRes = g_Engine->Run();
			if (!runRes.has_value()) {
				DOutError("Failed to initialized EditorDLL: {}.", runRes.error());
				return false;
			}

			return true;
		}
		catch (const std::exception& e)
		{
			DOutException("Exception during EditorDLL initialization: {}", e.what());
			return false;
		}
		catch (...)
		{
			DOutError("Unknown exception during EditorDLL Initialization");
			return false;
		}
	}

	EDITOR_API void Deinitialize()
	{
		if (g_Engine)
		{
			g_Engine.reset();
			DOut("EditorDLL deinitialized.");
		}
	}

	EDITOR_API void Tick()
	{
		try
		{
			if (g_Engine)
			{
				g_Engine->Tick();
			}
			else
				DOutWarning("Tick: g_Engine == nullptr.");
		}
		catch (const std::exception& e)
		{
			DOutException("Exception during Tick: {}", e.what());
		}
	}

	EDITOR_API void ClearEngine()
	{
		try
		{
			if (g_Engine)
			{
				g_Engine->ClearEngine();
				DOut("ClearEngine: Successed.");
			}
			else
				DOutWarning("ClearEngine: g_Engine == nullptr.");
		}
		catch (const std::exception& e)
		{
			DOutException("Exception during ClearEngine: {}", e.what());
		}
	}

	EDITOR_API void* AddView(void* hwnd)
	{
		try
		{
			if (g_Engine)
			{
				DOut("AddView({}). START.", hwnd);
				return g_Engine->AddView(hwnd);
			}
			else
				DOutWarning("AddView: g_Engine == nullptr.");
		}
		catch (const std::exception& e)
		{
			DOutException("Exception during AddView: {}", e.what());
		}
		return nullptr;
	}

	EDITOR_API void RemoveView(void* view)
	{
		try
		{
			if (g_Engine)
			{
				g_Engine->RemoveView(view);
			}
			else
				DOutWarning("RemoveView: g_Engine == nullptr.");
		}
		catch (const std::exception& e)
		{
			DOutException("Exception during RemoveView: {}", e.what());
		}
	}

	EDITOR_API void SetProjectPath(const char* projectPath)
	{
		try
		{
			if (g_Engine && projectPath)
			{
				g_Engine->SetProjectPath(projectPath);
			}
		}
		catch (const std::exception& e)
		{
			DOutException("Exception during SetProjectPath: {}", e.what());
		}
	}

	EDITOR_API void ReloadScripts()
	{
		try
		{
			if (g_Engine)
			{
				g_Engine->ReloadScripts();
			}
		}
		catch (const std::exception& e)
		{
			DOutException("Exception during ReloadScripts: {}", e.what());
		}
	}
}