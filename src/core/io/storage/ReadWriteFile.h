#pragma once

#include <span>
#include <mutex>
#include <string>
#include <vector>
#include <cstddef>
#include <expected>
#include <filesystem>

#include "core/utils/Macroses.h"

namespace zzz::core
{
	/**
	 * @brief Изменяемый файл с потокобезопасными операциями чтения и записи.
	 * @details Права на путь проверяет FileSystem при выдаче пути; конструктор проверок не выполняет.
	 *          Хэндл между вызовами не удерживается: файл открывается и закрывается внутри Read/Write.
	 *          Read и Write выполняют минимальные проверки при работе. Write с пустыми данными ничего не делает
	 *          (только предупреждение в лог), иначе при необходимости создаёт каталоги
	 *          и пишет напрямую в файл. При ошибке созданные файл и каталоги удаляются.
	 *          Ошибки Read/Write возвращаются через std::expected с логированием через UNEXPECTED.
	 */
	class ReadWriteFile final
	{
	public:
		Z_NO_COPY_MOVE(ReadWriteFile);

		explicit ReadWriteFile(const std::filesystem::path& physicalPath);

		[[nodiscard]] std::expected<std::vector<std::byte>, std::string> Read();
		std::expected<void, std::string> Write(std::span<const std::byte> bytes);

	private:
		std::mutex				m_Mutex;
		std::filesystem::path	m_Path;

#pragma region Validation helpers
		/// @brief Возвращает статус пути. Отсутствие файла ошибкой не считается.
		[[nodiscard]] static std::expected<std::filesystem::file_status, std::string> GetStatus(const std::filesystem::path& path);

		/// @brief Проверяет, что существующий путь — обычный файл, доступный на чтение и запись. Содержимое не изменяется.
		[[nodiscard]] static std::expected<void, std::string> CheckExistingFile(
			const std::filesystem::path& path, const std::filesystem::file_status& status);

		/// @brief Возвращает несуществующие каталоги-предки файла, от самого глубокого к корню.
		[[nodiscard]] static std::expected<std::vector<std::filesystem::path>, std::string> CollectMissingDirs(const std::filesystem::path& path);
#pragma endregion // Validation helpers
	};
}
