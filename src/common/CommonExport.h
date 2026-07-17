#pragma once

// Скрипты компилируются в отдельный scripts.dll и линкуются против editor_dll.lib (Hot-Reload,
// см. docs/scripting.md) - common_lib_editor пересекает границу DLL только в этом сценарии
// (Z_EDITOR), когда шаблоны из common/templates (например Event<>::Subscribe) инстанцируются уже
// в scripts.dll и вызывают функции common_lib (throw_runtime_error). В статической сборке игры
// (game_win/macos/linux/ios/android) Z_EDITOR не определён, common_lib линкуется статически -
// макрос должен быть пустым. См. также аналогичный макрос
// src/engine/public/core/userscripts/EngineExport.h.
#if defined(_WIN32) && defined(Z_EDITOR)
	#if defined(EDITORDLL_EXPORTS) || defined(editor_dll_EXPORTS)
		#define Z_COMMON_API __declspec(dllexport)
	#else
		#define Z_COMMON_API __declspec(dllimport)
	#endif
#else
	#define Z_COMMON_API
#endif
