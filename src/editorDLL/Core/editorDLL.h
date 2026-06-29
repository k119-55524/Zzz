#pragma once

#include <logger/logger.h>

#if defined(_WIN32)
#define EDITOR_API __declspec(dllexport)
#else
#define EDITOR_API
#endif

extern "C"
{
	EDITOR_API bool Initialize(zzz::logger::LogCallback callback);
	EDITOR_API void Deinitialize();
	EDITOR_API void Tick();
	EDITOR_API void ClearEngine();
	EDITOR_API void* AddView(void* hwnd);
	EDITOR_API void RemoveView(void* view);
	EDITOR_API void SetProjectPath(const char* projectPath);
	EDITOR_API void ReloadScripts();
}
