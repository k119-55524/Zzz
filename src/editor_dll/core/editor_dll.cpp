
#define Z_PRINT_DEFINES
#ifdef Z_PRINT_DEFINES
#endif
#include <memory>
#include <core/Core.h>
#include <engine/Engine.h>
#include "../engine_wrapper/EditorEngine.h"

#include "editor_dll.h"

std::unique_ptr<zzz::editor::EditorEngine> g_Engine;
zzz::logger::LogCallback g_EditorLogCallback = nullptr;

// Эта функция никогда не вызывается, но нужна, чтобы линкер не выбросил throw_runtime_error.
// Так как шаблон EventImpl<1>::Subscribe инстанцируется в scripts.dll, он требует экспорта 
// этой функции из editor_dll.dll. Из-за багов MSBuild с /WHOLEARCHIVE это самый надежный способ.
#pragma comment(linker, "/EXPORT:?throw_runtime_error@core@zzz@@YAXAEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEBUsource_location@4@@Z")
extern "C" __declspec(dllexport) void ForceExport_ThrowRuntimeError()
{
	auto dummyPtr = reinterpret_cast<void(*)(const std::string&, const std::source_location&)>(&zzz::core::throw_runtime_error);
	volatile void* force = reinterpret_cast<void*>(dummyPtr);
	(void)force;
}

extern "C"
{
	EDITOR_API bool Initialize(zzz::logger::LogCallback callback)
	{
		try
		{
			g_EditorLogCallback = callback;
#if Z_ADD_LOGGER || Z_DEVELOPMENT_BUILD
			zzz::logger::g_Logger.AddCallbackBroadcaster(callback);
#endif

			g_Engine = zzz::core::safe_make_unique<zzz::editor::EditorEngine>("ZzzEditorWin");
			DOut("EditorDLL инициализирован: успешно.");

			auto runRes = g_Engine->Run();
			if (!runRes.has_value())
			{
				DOutError("Не удалось инициализировать EditorDLL: {}.", runRes.error());
				return false;
			}

			return true;
		}
		catch (const std::exception& e)
		{
			DOutException("Исключение при инициализации EditorDLL: {}", e.what());
			return false;
		}
		catch (...)
		{
			DOutError("Неизвестное исключение при инициализации EditorDLL");
			return false;
		}
	}

	EDITOR_API void Deinitialize()
	{
		if (g_Engine)
		{
			g_Engine.reset();
			DOut("EditorDLL деинициализирован.");
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
			DOutException("Исключение во время Tick: {}", e.what());
		}
	}

	EDITOR_API void ClearEngine()
	{
		try
		{
			if (g_Engine)
			{
				g_Engine->ClearEngine();
				DOut("ClearEngine: успешно.");
			}
			else
				DOutWarning("ClearEngine: g_Engine == nullptr.");
		}
		catch (const std::exception& e)
		{
			DOutException("Исключение во время ClearEngine: {}", e.what());
		}
	}

	EDITOR_API void* AddView(void* hwnd)
	{
		try
		{
			if (g_Engine)
			{
				DOut("AddView({}). Начало.", hwnd);
				return g_Engine->AddView(hwnd);
			}
			else
				DOutWarning("AddView: g_Engine == nullptr.");
		}
		catch (const std::exception& e)
		{
			DOutException("Исключение во время AddView: {}", e.what());
		}
		return nullptr;
	}

	EDITOR_API void RemoveView(void* view)
	{
		try
		{
			if (g_Engine)
				g_Engine->RemoveView(view);
			else
				DOutWarning("RemoveView: g_Engine == nullptr.");
		}
		catch (const std::exception& e)
		{
			DOutException("Исключение во время RemoveView: {}", e.what());
		}
	}

	EDITOR_API void SetProjectPath(const char* projectPath)
	{
		try
		{
			if (g_Engine && projectPath)
				g_Engine->SetProjectPath(projectPath);
		}
		catch (const std::exception& e)
		{
			DOutException("Исключение во время SetProjectPath: {}", e.what());
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
			DOutException("Исключение во время ReloadScripts: {}", e.what());
		}
	}

	EDITOR_API void Play(const char** scriptClasses, int count)
	{
		try
		{
			if (g_Engine)
				g_Engine->Play(scriptClasses, count);
		}
		catch (const std::exception& e)
		{
			DOutException("Исключение во время Play: {}", e.what());
		}
	}

	EDITOR_API void Stop()
	{
		try
		{
			if (g_Engine)
				g_Engine->Stop();
		}
		catch (const std::exception& e)
		{
			DOutException("Исключение во время Stop: {}", e.what());
		}
	}

	EDITOR_API void Pause(bool isPaused)
	{
		try
		{
			if (g_Engine)
				g_Engine->Pause(isPaused);
		}
		catch (const std::exception& e)
		{
			DOutException("Исключение во время Pause: {}", e.what());
		}
	}
}
