#pragma once

#include <filesystem>

namespace zzz::core
{
	/**
	 * @brief Платформенно-изолированные проверки прав доступа к файлам и каталогам.
	 * @details Реализация выбирается на этапе сборки через CMake без макросов в заголовке.
	 *          Проверки ничего не создают и не изменяют на диске.
	 */
	class FileAccess final
	{
	public:
		FileAccess() = delete;

		/// @brief Проверяет, что существующий файл доступен текущему процессу на чтение и запись.
		[[nodiscard]] static bool CanReadWrite(const std::filesystem::path& filePath) noexcept;

		/// @brief Проверяет, что в существующем каталоге текущий процесс может создать файл.
		[[nodiscard]] static bool CanCreateIn(const std::filesystem::path& dirPath) noexcept;
	};
}
