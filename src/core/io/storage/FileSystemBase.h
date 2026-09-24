#pragma once

#include <memory>
#include <string>
#include <expected>
#include <filesystem>
#include <string_view>

#include "Path.h"
#include "core/enums/eFileLocation.h"
#include "core/utils/NativeAppData.h"

namespace zzz::core
{
	/**
	 * @class FileSystemBase
	 * @brief Базовая реализация файловой системы для кроссплатформенного ввода-вывода.
	 *
	 * @details Отвечает за разрешение путей, песочницы ОС (User, Saves, Cache, Logs, App)
	 *          и метаданные файловой системы (проверка существования и удаление файлов).
	 *          Не выполняет ввод-вывод байт (это задача ReadOnlyFile и ReadWriteFile).
	 */
	class FileSystemBase
	{
	public:
		explicit FileSystemBase(std::shared_ptr<NativeAppData> nativeData = nullptr);
		virtual ~FileSystemBase() = default;

		[[nodiscard]] virtual bool FileExists(eFileLocation location, const std::filesystem::path& relativePath) const noexcept;
		[[nodiscard]] virtual std::expected<void, std::string> DeleteFile(eFileLocation location, const std::filesystem::path& relativePath) const noexcept;

		[[nodiscard]] static constexpr bool IsLocationWritable(eFileLocation location) noexcept
		{
			switch (location)
			{
			case eFileLocation::User:
			case eFileLocation::Saves:
			case eFileLocation::Cache:
			case eFileLocation::Logs:
				return true;
			case eFileLocation::App:
			default:
				return false;
			}
		}

		[[nodiscard]] std::expected<std::filesystem::path, std::string> ResolvePhysicalPath(
			eFileLocation location, const std::filesystem::path& relativePath) const noexcept;

		[[nodiscard]] std::expected<std::filesystem::path, std::string> GetGamePackagePath() const noexcept;
		[[nodiscard]] std::expected<std::filesystem::path, std::string> GetDataPackagePath() const noexcept;

		/**
		 * @brief Инициализирует каталог пользовательских данных и возвращает путь к файлу настроек пользователя.
		 * @details Путь проверяется на чтение и запись (для отсутствующего файла — на возможность создания).
		 */
		[[nodiscard]] std::expected<std::filesystem::path, std::string> GetUserConfigPath(std::string_view companyName, std::string_view appName);

	protected:
		std::shared_ptr<NativeAppData> m_NativeData;
		std::shared_ptr<Path>          m_Path;

	private:
		/**
		 * @brief Проверяет физический путь к файлу.
		 * @param physicalPath Полный путь к файлу.
		 * @param writable false — файл существует и является обычным файлом;
		 *                 true — существующий файл доступен на чтение и запись, а для отсутствующего
		 *                 ближайший существующий каталог-предок позволяет создать файл.
		 */
		[[nodiscard]] std::expected<void, std::string> ValidateFilePath(const std::filesystem::path& physicalPath, bool writable = false) const noexcept;
	};
}
