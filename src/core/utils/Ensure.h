#pragma once

#include <format>
#include <utility>
#include <stdexcept>
#include <string_view>
#include <source_location>

#include "Defines.h"
#include "ThrowWrappers.h"

namespace zzz::core
{

	/**
	 * @brief Универсальный шаблон ensure для типов, которые можно привести к bool.
	 *        Например, std::shared_ptr, std::unique_ptr, raw pointer и другие типы с operator bool().
	 *
	 * @tparam T Тип проверяемого значения.
	 * @param condition Проверяемое условие или указатель.
	 * @param message Сообщение об ошибке, если условие ложно.
	 * @param loc Местоположение вызова (автоматически заполняется std::source_location::current()).
	 *
	 * Пример использования:
	 * @code
	 * std::shared_ptr<PlatfotmConfig> appConfig;
	 * ensure(appConfig, "PlatfotmConfig не должен быть null");
	 * @endcode
	 */
	template<typename T>
	inline void ensure(
		[[maybe_unused]] T&& condition,
		[[maybe_unused]] std::string_view message = "Указатель не должен быть null",
		[[maybe_unused]] const std::source_location& loc = std::source_location::current())
	{
#if Z_DEBUG_BUILD || Z_DEVELOPMENT_BUILD
		if (condition) [[likely]]
			return;

		throw_ensure(message, loc);
#endif
	}

	/**
	 * @brief Overload ensure для явных bool-условий.
	 *
	 * @param condition Логическое условие.
	 * @param message Сообщение об ошибке, если условие ложно.
	 * @param loc Местоположение вызова.
	 *
	 * Пример использования:
	 * @code
	 * ensure(x > 0, "x должен быть положительным");
	 * @endcode
	 */
	inline void ensure(
		[[maybe_unused]] bool condition,
		[[maybe_unused]] std::string_view message = "Условие не выполнено",
		[[maybe_unused]] const std::source_location& loc = std::source_location::current())
	{

#if Z_DEBUG_BUILD || Z_DEVELOPMENT_BUILD
		if (condition) [[likely]]
			return;

		throw_ensure(message, loc);
#endif
	}

	/**
	 * @brief Overload ensure с поддержкой форматируемых сообщений через std::format.
	 *
	 * @tparam Args Типы аргументов для форматирования.
	 * @param condition Логическое условие.
	 * @param fmt Форматируемая строка.
	 * @param args Аргументы для подстановки в форматируемую строку.
	 *
	 * Пример использования:
	 * @code
	 * ensure(x > 0, "Некорректное значение x={}, y={}", x, y);
	 * @endcode
	 */
	template<typename... Args>
	inline void ensure(
		[[maybe_unused]] bool condition,
		[[maybe_unused]] std::format_string<Args...> fmt,
		[[maybe_unused]] Args&&... args)
	{

#if Z_DEBUG_BUILD || Z_DEVELOPMENT_BUILD
		if (condition) [[likely]]
			return;

		throw_ensure(std::format(fmt, std::forward<Args>(args)...), std::source_location::current());
#endif
	}
}