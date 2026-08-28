#pragma once

#include <core/CoreIncludes.h>
#include "core/utils/macros/LogMacros.h" // UNEXPECTED использует DOutError

/**
 * @brief Выбрасывает std::runtime_error с форматированным сообщением и текущей позицией в коде (файл, строка).
 */
#define THROW_RUNTIME(...) ::zzz::core::throw_runtime_error(std::format(__VA_ARGS__), std::source_location::current())

/// @brief Проверяет валидность функтора (в Debug/Development) и вызывает его.
#if Z_DEBUG_BUILD || Z_DEVELOPMENT_BUILD
#define VERIFY_AND_CALL(func, ...) \
	do \
	{ \
		if (!(func)) \
			THROW_RUNTIME("Функтор '{}' не назначен.", #func); \
		(func)(__VA_ARGS__); \
	} while (false)
#else
#define VERIFY_AND_CALL(func, ...) (func)(__VA_ARGS__)
#endif

/// @brief Макрос для возврата std::unexpected с логированием ошибки.
#define UNEXPECTED(fmt, ...) \
    ([&]() { \
        auto msg = std::format(fmt, ##__VA_ARGS__); \
        DOutError("{}", msg); \
        return std::unexpected(std::move(msg)); \
    }())

/// @brief Запрещает копирование класса.
#define Z_NO_COPY(ClassName) \
	ClassName(const ClassName&) = delete; \
	ClassName& operator=(const ClassName&) = delete

/// @brief Запрещает перемещение класса.
#define Z_NO_MOVE(ClassName) \
	ClassName(ClassName&&) noexcept = delete; \
	ClassName& operator=(ClassName&&) noexcept = delete

/// @brief Запрещает копирование и перемещение класса.
#define Z_NO_COPY_MOVE(ClassName) \
	ClassName(const ClassName&) = delete; \
	ClassName& operator=(const ClassName&) = delete; \
	ClassName(ClassName&&) = delete; \
	ClassName& operator=(ClassName&&) = delete

#if Z_WINDOWS
#if Z_DEBUG_BUILD || Z_DEVELOPMENT_BUILD
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

/**
 * @brief Начало блока проверки утечек памяти (CRT). Только для Windows.
 */
#define CRT_LEAK_CHECK_BEGIN(...) \
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF)

/**
 * @brief Конец блока проверки утечек памяти. При использовании автоматического флага возвращает 0.
 *        Реальные утечки будут выведены в Output окно студии после завершения процесса.
 */
#define CRT_LEAK_CHECK_END() 0
#else
#define CRT_LEAK_CHECK_BEGIN(...)
#define CRT_LEAK_CHECK_END() 0
#endif
#endif
