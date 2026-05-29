#pragma once

#include "../header.h"

namespace zzz::io
{
	class Path final
	{
	public:
		Path() = delete;

		static std::filesystem::path GetExecutableDirectory();
		static char GetSeparator();

		/**
		 * @brief Проверяет корректность пути.
		 *
		 * Выполняет базовую проверку пути и его компонентов
		 * в соответствии с правилами текущей платформы.
		 *
		 * @param path Проверяемый путь.
		 *
		 * @return true, если путь считается корректным.
		 * @return false, если путь некорректен.
		 */
		static bool IsValidPath(std::string_view path);

		/**
		 * @brief Проверяет строку пути на наличие недопустимых символов.
		 *
		 * Метод выполняет синтаксическую проверку строки без
		 * обращения к файловой системе. Наличие файла или директории
		 * по указанному пути не проверяется.
		 *
		 * @param path Проверяемая строка пути.
		 *
		 * @return true, если строка не содержит недопустимых символов.
		 * @return false, если строка содержит недопустимые символы
		 *         или не может быть использована как путь.
		 */
		static bool IsPathStringValid(std::string_view path);
	};
}
