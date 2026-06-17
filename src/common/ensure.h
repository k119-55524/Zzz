#pragma once

#include "defines.h"
#include <format>
#include <utility>
#include <stdexcept>
#include <string_view>
#include <source_location>

namespace zzz::common
{
	/**
	 * @brief Вспомогательная функция для генерации исключения std::runtime_error с информацией о месте вызова.
	 *
	 * @param message Сообщение об ошибке.
	 * @param loc Местоположение вызова (автоматически заполняется std::source_location::current()).
	 * @throws std::runtime_error Сформированное исключение с информацией о файле, функции и строке.
	 */
	[[noreturn]]
	inline void throw_ensure(
		std::string_view message,
		const std::source_location& loc)
	{
		throw std::runtime_error(
			std::format(
				">>>> [{}]\nLine: {}\nFile: {}\n\n{}",
				loc.function_name(),
				loc.line(),
				loc.file_name(),
				message));
	}

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
	 * ensure(appConfig, "PlatfotmConfig cannot be null");
	 * @endcode
	 */
	template<typename T>
	inline void ensure(
		T&& condition,
		std::string_view message = "Pointer must not be null",
		const std::source_location& loc = std::source_location::current())
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
	 * ensure(x > 0, "x must be positive");
	 * @endcode
	 */
	inline void ensure(
		bool condition,
		std::string_view message = "Ensure failed",
		const std::source_location& loc = std::source_location::current())
	{
		// Подавляем предупреждения о неиспользуемых параметрах
		// Чтобы не засорять вывод компилятора
		(void)condition;
		(void)message;
		(void)loc;

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
	 * ensure(x > 0, "Invalid value x={}, y={}", x, y);
	 * @endcode
	 */
	template<typename... Args>
	inline void ensure(
		bool condition,
		std::format_string<Args...> fmt,
		Args&&... args)
	{
		// Подавляем предупреждения о неиспользуемых параметрах
		// Чтобы не засорять вывод компилятора
		(void)condition;
		(void)fmt;
		((void)args, ...);

#if Z_DEBUG_BUILD || Z_DEVELOPMENT_BUILD
		if (condition) [[likely]]
			return;

		throw_ensure(std::format(fmt, std::forward<Args>(args)...), std::source_location::current());
#endif
	}
}