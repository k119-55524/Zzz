#pragma once

#include <source_location>
#include <core/CoreIncludes.h>
#include "core/utils/LogCategory.h"

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

	// 1. Throttled log overload (First пользовательский аргумент после cat - int или float)
	template<typename ShouldLogFunc, typename LogFunc, typename Throttle, typename Fmt, typename... Args>
	requires (std::is_arithmetic_v<std::decay_t<Throttle>> && !std::is_same_v<std::decay_t<Throttle>, bool> && !std::is_same_v<std::decay_t<Throttle>, char>)
	inline void LogDispatchImpl(ShouldLogFunc&& shouldLogFunc, LogFunc&& logFunc, const std::source_location& loc, const ::zzz::core::LogCategory& cat, Throttle throttleVal, Fmt&& fmt, Args&&... args)
	{
		// Early Exit до форматирования/аллокации - категория проверяется первой (см. shouldLogFunc в Z_LOG_DISPATCH).
		if (!shouldLogFunc(cat))
			return;

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
					logFunc(loc, cat, FormatMsg(std::forward<Fmt>(fmt), std::forward<Args>(args)...));
				}
			}
		}
		else if constexpr (std::is_integral_v<T>)
		{
			uint64_t n = static_cast<uint64_t>(throttleVal);
			if (n <= 1 || (state.counter.fetch_add(1, std::memory_order_relaxed) % n) == 0)
			{
				logFunc(loc, cat, FormatMsg(std::forward<Fmt>(fmt), std::forward<Args>(args)...));
			}
		}
	}

	// 2. Conditional log overload (First пользовательский аргумент после cat - bool condition)
	template<typename ShouldLogFunc, typename LogFunc, typename Fmt, typename... Args>
	inline void LogDispatchImpl(ShouldLogFunc&& shouldLogFunc, LogFunc&& logFunc, const std::source_location& loc, const ::zzz::core::LogCategory& cat, bool condition, Fmt&& fmt, Args&&... args)
	{
		if (!shouldLogFunc(cat))
			return;

		if (!condition)
			return;

		auto FormatMsg = [](auto&& f, auto&&... a) {
			if constexpr (sizeof...(a) == 0)
				return std::string(std::forward<decltype(f)>(f));
			else
				return std::vformat(std::string_view(f), std::make_format_args(a...));
		};

		logFunc(loc, cat, FormatMsg(std::forward<Fmt>(fmt), std::forward<Args>(args)...));
	}

	// 3. Regular log overload (First пользовательский аргумент после cat - формат-строка)
	template<typename ShouldLogFunc, typename LogFunc, typename Fmt, typename... Args>
	requires (!std::is_arithmetic_v<std::decay_t<Fmt>> || std::is_same_v<std::decay_t<Fmt>, char>)
	inline void LogDispatchImpl(ShouldLogFunc&& shouldLogFunc, LogFunc&& logFunc, const std::source_location& loc, const ::zzz::core::LogCategory& cat, Fmt&& fmt, Args&&... args)
	{
		if (!shouldLogFunc(cat))
			return;

		auto FormatMsg = [](auto&& f, auto&&... a) {
			if constexpr (sizeof...(a) == 0)
				return std::string(std::forward<decltype(f)>(f));
			else
				return std::vformat(std::string_view(f), std::make_format_args(a...));
		};

		logFunc(loc, cat, FormatMsg(std::forward<Fmt>(fmt), std::forward<Args>(args)...));
	}

	// --- Выбор категории: явная (первый пользовательский аргумент - LogCategory) или ambient (CurrentFileLogCategory) ---
	//
	// shouldLogFunc/logFunc - лямбды, определяемые прямо в месте вызова макроса (Z_LOG_DISPATCH), где тип Logger
	// уже переданются сюда как шаблонные (dependent) параметры специально, чтобы
	// сама LogMacros.h не требовала полного типа Logger - это низкоуровневый заголовок, включаемый повсеместно
	// (в т.ч. в единицах трансляции core_lib, где logger.h не подключён).

	// A. Явная категория передана первым аргументом макроса: DOut(LogGAPI, "...", ...)
	template<typename ShouldLogFunc, typename LogFunc, typename... Rest>
	inline void LogDispatchCategory(ShouldLogFunc&& shouldLogFunc, LogFunc&& logFunc, const std::source_location& loc, const ::zzz::core::LogCategory& /*ambientCat*/, const ::zzz::core::LogCategory& explicitCat, Rest&&... rest)
	{
		LogDispatchImpl(std::forward<ShouldLogFunc>(shouldLogFunc), std::forward<LogFunc>(logFunc), loc, explicitCat, std::forward<Rest>(rest)...);
	}

	// B. Категория не передана явно - используется ambient CurrentFileLogCategory: DOut("...", ...)
	template<typename ShouldLogFunc, typename LogFunc, typename First, typename... Rest>
	requires (!std::is_same_v<std::decay_t<First>, ::zzz::core::LogCategory>)
	inline void LogDispatchCategory(ShouldLogFunc&& shouldLogFunc, LogFunc&& logFunc, const std::source_location& loc, const ::zzz::core::LogCategory& ambientCat, First&& first, Rest&&... rest)
	{
		LogDispatchImpl(std::forward<ShouldLogFunc>(shouldLogFunc), std::forward<LogFunc>(logFunc), loc, ambientCat, std::forward<First>(first), std::forward<Rest>(rest)...);
	}

	// --- Ambient категория файла (CurrentFileLogCategory / Z_SET_LOG_CATEGORY) ---
	//
	// z_TU_Tag - уникальный (за счёт анонимного namespace) тип на каждую единицу трансляции. CategoryHolder<z_TU_Tag>
	// хранит категорию по умолчанию для файла (LogGeneral) через первичный шаблон, а Z_SET_LOG_CATEGORY(Category)
	// переопределяет её явной специализации CategoryHolder<z_TU_Tag>. Т.к явная специализация одного и того же
	// шаблона дважды в одной единице трансляции - жёсткая ошибка компиляции (redefinition), повторный вызов
	// Z_SET_LOG_CATEGORY в одном файле гарантированно не скомпилируется.
	//
	// ВАЖНО: Z_SET_LOG_CATEGORY(...) должен стоять в файле раньше первого использования DOut*/CurrentFileLogCategory -
	// иначе первичный шаблон будет неявно инстанциирован до специализации (ошибка "explicit specialization after instantiation").
	namespace detail
	{
		namespace { struct z_TU_Tag; }

		template<typename Tag>
		struct CategoryHolder
		{
			static constexpr const ::zzz::core::LogCategory& value = ::zzz::core::LogGeneral;
		};
	}
}

/**
 * @brief Ambient-категория текущего файла: LogGeneral по умолчанию, либо категория, заданная через Z_SET_LOG_CATEGORY.
 */
#define CurrentFileLogCategory (::zzz::logger::detail::CategoryHolder<::zzz::logger::detail::z_TU_Tag>::value)

/**
 * @brief Задаёт ambient-категорию логирования (CurrentFileLogCategory) для текущего .cpp файла.
 * @details Должен быть указан один раз в файле, до первого использования DOut*-макросов или CurrentFileLogCategory (обычно
 * сразу после #include). Повторный вызов в одном файле - ошибка компиляции (redefinition).
 */
#define Z_SET_LOG_CATEGORY(Category) \
	namespace zzz::logger::detail { \
		template<> struct CategoryHolder<z_TU_Tag> { static constexpr const ::zzz::core::LogCategory& value = (Category); }; \
	}

// Общее тело DOut*. Loc - параметр, а не current() внутри, чтобы можно было передать заранее вычисленный source_location.
// Категория берётся явно (если первый аргумент - LogCategory) либо из ambient CurrentFileLogCategory файла.
//
// ApplyFilter=true (только у DOut/Message) - реально проверяет g_Logger.IsCategoryEnabled(cat) до форматирования
// (Early Exit). ApplyFilter=false (Warning/Error/Exception/Critical/Fatal) - лямбда-предикат вырождается в
// константу true и IsCategoryEnabled даже не вызывается (короткое замыкание "!(false) || ..."): это и есть
// гарантированная (не фильтруемая рантаймо) доставка этих уровней, см. правило "г" плана категорий логирования.
//
// Само определение вне #if Z_ADD_LOGGER - инфраструктура категорий должна существовать независимо от гейта логера.
#define Z_LOG_DISPATCH(LogFn, ApplyFilter, Loc, ...) \
	do { \
		::zzz::logger::LogDispatchCategory( \
			[](const ::zzz::core::LogCategory& cat) -> bool { return !(ApplyFilter) || ::zzz::logger::g_Logger.IsCategoryEnabled(cat); }, \
			[](const std::source_location& loc, const ::zzz::core::LogCategory& cat, std::string msg) { \
				::zzz::logger::g_Logger.LogFn(loc, cat, std::move(msg)); \
			}, Loc, CurrentFileLogCategory, __VA_ARGS__); \
	} while (false)

#if Z_ADD_LOGGER
/**
 * @brief Вывод информационного сообщения в лог.
 * @details Категория - первым аргументом (DOut(LogGAPI, "...")), либо ambient CurrentFileLogCategory файла.
 * Единственный уровень, подверженный рантайм-фильтрации по категории (см. Logger::IsCategoryEnabled).
 */
#define DOut(...) Z_LOG_DISPATCH(LogMessage, true, std::source_location::current(), __VA_ARGS__)

/**
 * @brief Вывод предупреждения в лог. Гарантированная доставка - категория не фильтруется.
 */
#define DOutWarning(...) Z_LOG_DISPATCH(LogWarning, false, std::source_location::current(), __VA_ARGS__)

/**
 * @brief Вывод ошибки в лог. Гарантированная доставка - категория не фильтруется.
 */
#define DOutError(...) Z_LOG_DISPATCH(LogError, false, std::source_location::current(), __VA_ARGS__)

/**
 * @brief Вывод исключения в лог. Гарантированная доставка - категория не фильтруется.
 */
#define DOutException(...) Z_LOG_DISPATCH(LogException, false, std::source_location::current(), __VA_ARGS__)

/**
 * @brief Вывод критической ошибки в лог. Гарантированная доставка - категория не фильтруется.
 */
#define DOutCritical(...) Z_LOG_DISPATCH(LogCritical, false, std::source_location::current(), __VA_ARGS__)

/**
 * @brief Вывод фатальной ошибки в лог с последующим завершением процесса. Гарантированная доставка.
 */
#define DOutFatal(...) Z_LOG_DISPATCH(LogFatal, false, std::source_location::current(), __VA_ARGS__)

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
