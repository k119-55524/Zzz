#pragma once

// Скрипты компилируются в отдельный scripts.dll и линкуются против editor_dll.lib (Hot-Reload,
// см. docs/scripting.md) - только в этом сценарии (Z_EDITOR) engine_lib пересекает границу DLL.
// В статической сборке игры (game_win/macos/linux/ios/android) Z_EDITOR не определён, engine_lib
// линкуется статически - макрос должен быть пустым, иначе линковщик ждал бы несуществующий импорт.
#if defined(_WIN32) && defined(Z_EDITOR)
	#if defined(EDITORDLL_EXPORTS) || defined(editor_dll_EXPORTS)
		#define Z_ENGINE_API __declspec(dllexport)
	#else
		#define Z_ENGINE_API __declspec(dllimport)
	#endif
#else
	#define Z_ENGINE_API
#endif
