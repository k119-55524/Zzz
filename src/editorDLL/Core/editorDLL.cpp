
#include <memory>
#include <common/common.h>
#include <engine/engine.h>
#include "../EngineWrapper/EditorEngine.h"

#include "editorDLL.h"

std::unique_ptr<zzz::editor::EditorEngine> g_Engine;

extern "C"
{
	EDITOR_API bool Initialize(void* hwnd)
	{
		try
		{
			auto data = std::make_shared<zzz::engine::NativeAppData>();
			data->hwnd = static_cast<HWND>(hwnd);

			g_Engine = std::make_unique<zzz::editor::EditorEngine>("ZzzEditorWin", data);

			DOut("ZzzEditorDLL initialized.");

			auto runRes = g_Engine->Run();
			if (!runRes.has_value()) {
				DOutError("Failed to Run Engine in Editor DLL: {}.", runRes.error());
				return false;
			}

			return true;
		}
		catch (const std::exception& e)
		{
			DOutException("Exception during ZzzEditorDLL Initialization: {}", e.what());
			return false;
		}
		catch (...)
		{
			DOutError("Unknown exception during ZzzEditorDLL Initialization");
			return false;
		}
	}

	EDITOR_API void Deinitialize()
	{
		if (g_Engine)
		{
			g_Engine.reset();
			DOut("ZzzEditorDLL deinitialized.");
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
		}
		catch (const std::exception& e)
		{
			DOutException("Exception during ZzzEditorDLL Tick: {}", e.what());
		}
	}
}