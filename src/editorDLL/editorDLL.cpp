
#include <memory>
#include <common/common.h>
#include <engine/engine.h>

#include "editorDLL.h"

std::unique_ptr<zzz::engine::Engine> g_Engine;

extern "C"
{
	EDITOR_API bool Initialize(void* hwnd)
	{
		try
		{
			auto data = std::make_shared<zzz::engine::NativeAppData>();
			data->hwnd = static_cast<HWND>(hwnd);
			
			g_Engine = std::make_unique<zzz::engine::Engine>("ZzzEditorWin", data);
			
			auto res = g_Engine->Initialize();
			if (!res.has_value()) {
				DOutError("Editor engine initialize error: {}.", res.error());
				return false;
			}
			
			DOut("Editor engine initialize success.");
			return true;
		}
		catch (const std::exception& e)
		{
			DOutException("EditorDLL Initialize exception: {}.", e.what());
			return false;
		}
		catch (...)
		{
			DOutException("EditorDLL Initialize unknown exception.");
			return false;
		}
	}

	EDITOR_API void Deinitialize()
	{
		DOut("Editor engine deinitialize.");
		g_Engine.reset();
	}
}
