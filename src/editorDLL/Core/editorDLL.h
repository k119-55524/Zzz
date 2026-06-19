#pragma once

#if defined(_WIN32)
#define EDITOR_API __declspec(dllexport)
#else
#define EDITOR_API
#endif

extern "C"
{
	EDITOR_API bool Initialize(void* hwnd);
	EDITOR_API void Deinitialize();
	EDITOR_API void Tick();
}
