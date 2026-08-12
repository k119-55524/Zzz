#pragma once

// Скрипты компилируются в отдельный scripts.dll и линкуются против editor_dll.lib (Hot-Reload,
// см. docs/ARCHITECTURE.md) - core_lib_editor пересекает границу DLL только в этом сценарии
// (Z_EDITOR), когда шаблоны из core/templates инстанцируются уже
// в scripts.dll и вызывают функции core_lib (throw_runtime_error). В статической сборке игры
// (game_win/macos/linux/ios/android) Z_EDITOR не определён, core_lib линкуется статически -
// макрос должен быть пустым. См. также аналогичный макрос
// core/utils/Export.h.
#if defined(_WIN32) && defined(Z_EDITOR)
	#if defined(EDITORDLL_EXPORTS) || defined(editor_dll_EXPORTS)
		#define Z_CORE_API __declspec(dllexport)
	#else
		#define Z_CORE_API __declspec(dllimport)
	#endif
#else
	#define Z_CORE_API
#endif
