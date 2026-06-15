#pragma once

#include <expected>
#include <stdexcept>
#include "platform_defines.h"
#include <format>

#if Z_ADD_LOGGER || Z_DEVELOPMENT_BUILD
#define DOut(...) \
	do { \
		if (!::zzz::logger::g_Logger) throw std::runtime_error("Logger is not initialized!"); \
		::zzz::logger::g_Logger->LogMessage(std::source_location::current(), std::format(__VA_ARGS__)); \
	} while (false)
#define DOutWarning(...) \
	do { \
		if (!::zzz::logger::g_Logger) throw std::runtime_error("Logger is not initialized!"); \
		::zzz::logger::g_Logger->LogWarning(std::source_location::current(), std::format(__VA_ARGS__)); \
	} while (false)
#define DOutError(...) \
	do { \
		if (!::zzz::logger::g_Logger) throw std::runtime_error("Logger is not initialized!"); \
		::zzz::logger::g_Logger->LogError(std::source_location::current(), std::format(__VA_ARGS__)); \
	} while (false)
#define DOutException(...) \
	do { \
		if (!::zzz::logger::g_Logger) throw std::runtime_error("Logger is not initialized!"); \
		::zzz::logger::g_Logger->LogException(std::source_location::current(), std::format(__VA_ARGS__)); \
	} while (false)
#define DOutCritical(...) \
	do { \
		if (!::zzz::logger::g_Logger) throw std::runtime_error("Logger is not initialized!"); \
		::zzz::logger::g_Logger->LogCritical(std::source_location::current(), std::format(__VA_ARGS__)); \
	} while (false)
#else
#define DOut(...)
#define DOutWarning(...)
#define DOutError(...)
#define DOutException(...)
#define DOutCritical(...)
#endif

#define THROW_RUNTIME(...) ::zzz::throw_runtime_error(std::format(__VA_ARGS__), std::source_location::current())

/// @brief Проверяет валидность функтора (в Debug/Development) и вызывает его.
#if Z_DEBUG_BUILD || Z_DEVELOPMENT_BUILD
#define VERIFY_AND_CALL(func, ...) \
	do \
	{ \
		if (!(func)) \
			THROW_RUNTIME("Functor '{}' is not assigned.", #func); \
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
#define CRT_LEAK_CHECK_BEGIN(...) \
	_CrtMemState _crtLeakCtx{}; \
	_CrtMemCheckpoint(&_crtLeakCtx)
#define CRT_LEAK_CHECK_END() \
	([&]() -> int { \
		_CrtMemState _crtLeakCtxEnd{}; \
		_CrtMemState _crtLeakCtxDiff{}; \
		_CrtMemCheckpoint(&_crtLeakCtxEnd); \
		int leakFound = _CrtMemDifference(&_crtLeakCtxDiff, &_crtLeakCtx, &_crtLeakCtxEnd); \
		if (leakFound) { \
			_CrtMemDumpStatistics(&_crtLeakCtxDiff); \
			_CrtMemDumpAllObjectsSince(&_crtLeakCtx); \
			return -1; \
		} \
		return 0; \
	}())
#else
#define CRT_LEAK_CHECK_BEGIN(...)
#define CRT_LEAK_CHECK_END() 0
#endif
#endif
