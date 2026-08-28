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

// Общее тело DOut*/DOut*GAPI. Loc - параметр, а не current() внутри, чтобы GAPILogMacros.h мог передать
// пустой source_location (см. там). Само определение вне #if - нужно и под более широким гейтом GAPI.
#define Z_LOG_DISPATCH(LogFn, Loc, ...) \
	do { \
		::zzz::logger::LogDispatchImpl([](const std::source_location& loc, std::string msg) { \
			::zzz::logger::g_Logger.LogFn(loc, std::move(msg)); \
		}, Loc, __VA_ARGS__); \
	} while (false)

#if Z_ADD_LOGGER
/**
 * @brief Вывод информационного сообщения в лог.
 */
#define DOut(...) Z_LOG_DISPATCH(LogMessage, std::source_location::current(), __VA_ARGS__)

/**
 * @brief Вывод предупреждения в лог.
 */
#define DOutWarning(...) Z_LOG_DISPATCH(LogWarning, std::source_location::current(), __VA_ARGS__)

/**
 * @brief Вывод ошибки в лог.
 */
#define DOutError(...) Z_LOG_DISPATCH(LogError, std::source_location::current(), __VA_ARGS__)

/**
 * @brief Вывод исключения в лог.
 */
#define DOutException(...) Z_LOG_DISPATCH(LogException, std::source_location::current(), __VA_ARGS__)

/**
 * @brief Вывод критической ошибки в лог.
 */
#define DOutCritical(...) Z_LOG_DISPATCH(LogCritical, std::source_location::current(), __VA_ARGS__)

/**
 * @brief Вывод фатальной ошибки в лог с последующим завершением процесса.
 */
#define DOutFatal(...) Z_LOG_DISPATCH(LogFatal, std::source_location::current(), __VA_ARGS__)

#define Z_LOG_VAR(type, name, init)        type name{ init }
#define Z_LOG_GLOBAL_VAR(type, name, init) inline type name{ init }
#define Z_LOG_SET(name, val)               (name = (val))
#define Z_LOG_GET(name)                    (name)
#else // Z_ADD_LOGGER
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
#endif // Z_ADD_LOGGER
