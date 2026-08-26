#pragma once

#include <source_location>
#include <core/CoreIncludes.h>

namespace
{
	struct LogThrottleState
	{
		std::atomic<uint64_t> counter{ 0 };
		std::atomic<int64_t> lastTicks{ 0 };
	};	
}

namespace zzz::logger
{
	class Logger;
	extern Logger g_Logger;

	// 1. Throttled log overload (First arg after loc is int or float)
	template<typename LogFunc, typename Throttle, typename Fmt, typename... Args>
	requires (std::is_arithmetic_v<std::decay_t<Throttle>> && !std::is_same_v<std::decay_t<Throttle>, bool> && !std::is_same_v<std::decay_t<Throttle>, char>)
	inline void LogDispatchImpl(LogFunc&& logFunc, const std::source_location& loc, Throttle throttleVal, Fmt&& fmt, Args&&... args)
	{
		using T = std::decay_t<Throttle>;
		static LogThrottleState state;

		auto FormatMsg = [](auto&& f, auto&&... a) {
			if constexpr (sizeof...(a) == 0)
				return std::string(std::forward<decltype(f)>(f));
			else
				return std::vformat(std::string_view(f), std::make_format_args(a...));
		};

		if constexpr (std::is_floating_point_v<T>)
		{
			using clock = std::chrono::steady_clock;
			const auto nowTicks = clock::now().time_since_epoch().count();
			auto lastTicks = state.lastTicks.load(std::memory_order_relaxed);

			const double elapsedSec = (lastTicks == 0) ? 0.0 : std::chrono::duration<double>(clock::duration(nowTicks - lastTicks)).count();

			if (lastTicks == 0 || elapsedSec >= static_cast<double>(throttleVal))
			{
				if (state.lastTicks.compare_exchange_strong(lastTicks, nowTicks, std::memory_order_relaxed))
				{
					logFunc(loc, FormatMsg(std::forward<Fmt>(fmt), std::forward<Args>(args)...));
				}
			}
		}
		else if constexpr (std::is_integral_v<T>)
		{
			uint64_t n = static_cast<uint64_t>(throttleVal);
			if (n <= 1 || (state.counter.fetch_add(1, std::memory_order_relaxed) % n) == 0)
			{
				logFunc(loc, FormatMsg(std::forward<Fmt>(fmt), std::forward<Args>(args)...));
			}
		}
	}

	// 2. Conditional log overload (First arg after loc is bool condition)
	template<typename LogFunc, typename Fmt, typename... Args>
	inline void LogDispatchImpl(LogFunc&& logFunc, const std::source_location& loc, bool condition, Fmt&& fmt, Args&&... args)
	{
		if (!condition)
			return;

		auto FormatMsg = [](auto&& f, auto&&... a) {
			if constexpr (sizeof...(a) == 0)
				return std::string(std::forward<decltype(f)>(f));
			else
				return std::vformat(std::string_view(f), std::make_format_args(a...));
		};

		logFunc(loc, FormatMsg(std::forward<Fmt>(fmt), std::forward<Args>(args)...));
	}

	// 3. Regular log overload (First arg after loc is format string)
	template<typename LogFunc, typename Fmt, typename... Args>
	requires (!std::is_arithmetic_v<std::decay_t<Fmt>> || std::is_same_v<std::decay_t<Fmt>, char>)
	inline void LogDispatchImpl(LogFunc&& logFunc, const std::source_location& loc, Fmt&& fmt, Args&&... args)
	{
		auto FormatMsg = [](auto&& f, auto&&... a) {
			if constexpr (sizeof...(a) == 0)
				return std::string(std::forward<decltype(f)>(f));
			else
				return std::vformat(std::string_view(f), std::make_format_args(a...));
		};

		logFunc(loc, FormatMsg(std::forward<Fmt>(fmt), std::forward<Args>(args)...));
	}
}

#if Z_ADD_LOGGER || Z_DEVELOPMENT_BUILD
/**
 * @brief Вывод информационного сообщения в лог.
 */
#define DOut(...) \
	do { \
		::zzz::logger::LogDispatchImpl([](const std::source_location& loc, std::string msg) { \
			::zzz::logger::g_Logger.LogMessage(loc, std::move(msg)); \
		}, std::source_location::current(), __VA_ARGS__); \
	} while (false)

/**
 * @brief Вывод предупреждения в лог.
 */
#define DOutWarning(...) \
	do { \
		::zzz::logger::LogDispatchImpl([](const std::source_location& loc, std::string msg) { \
			::zzz::logger::g_Logger.LogWarning(loc, std::move(msg)); \
		}, std::source_location::current(), __VA_ARGS__); \
	} while (false)

/**
 * @brief Вывод ошибки в лог.
 */
#define DOutError(...) \
	do { \
		::zzz::logger::LogDispatchImpl([](const std::source_location& loc, std::string msg) { \
			::zzz::logger::g_Logger.LogError(loc, std::move(msg)); \
		}, std::source_location::current(), __VA_ARGS__); \
	} while (false)

/**
 * @brief Вывод исключения в лог.
 */
#define DOutException(...) \
	do { \
		::zzz::logger::LogDispatchImpl([](const std::source_location& loc, std::string msg) { \
			::zzz::logger::g_Logger.LogException(loc, std::move(msg)); \
		}, std::source_location::current(), __VA_ARGS__); \
	} while (false)

/**
 * @brief Вывод критической ошибки в лог.
 */
#define DOutCritical(...) \
	do { \
		::zzz::logger::LogDispatchImpl([](const std::source_location& loc, std::string msg) { \
			::zzz::logger::g_Logger.LogCritical(loc, std::move(msg)); \
		}, std::source_location::current(), __VA_ARGS__); \
	} while (false)

/**
 * @brief Вывод фатальной ошибки в лог с последующим завершением процесса.
 */
#define DOutFatal(...) \
	do { \
		::zzz::logger::LogDispatchImpl([](const std::source_location& loc, std::string msg) { \
			::zzz::logger::g_Logger.LogFatal(loc, std::move(msg)); \
		}, std::source_location::current(), __VA_ARGS__); \
	} while (false)

#define Z_LOG_VAR(type, name, init)        type name{ init }
#define Z_LOG_GLOBAL_VAR(type, name, init) inline type name{ init }
#define Z_LOG_SET(name, val)               (name = (val))
#define Z_LOG_GET(name)                    (name)
#else // Z_ADD_LOGGER || Z_DEVELOPMENT_BUILD
#define DOut(...)
#define DOutWarning(...)
#define DOutError(...)
#define DOutException(...)
#define DOutCritical(...)
#define DOutFatal(...)

#define Z_LOG_VAR(type, name, init)
#define Z_LOG_GLOBAL_VAR(type, name, init)
#define Z_LOG_SET(name, val)               ((void)0)
#define Z_LOG_GET(name)                    false
#endif // Z_ADD_LOGGER || Z_DEVELOPMENT_BUILD

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