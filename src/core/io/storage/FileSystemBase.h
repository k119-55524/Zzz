#pragma once

#include <cstdint>
#include <expected>
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>

#include "Path.h"
#include "core/enums/eFileLocation.h"
#include "core/utils/NativeAppData.h"

namespace zzz::engine
{
	class Engine;
}

namespace zzz::core
{
	class ReadOnlyFile;
	class ReadWriteFile;

	/**
	 * @class FileSystemBase
	 * @brief Базовая реализация файловой системы для кроссплатформенного ввода-вывода.
	 *
	 * @details Отвечает за разрешение путей, песочницы ОС (User, Saves, Cache, Logs, App)
	 *          и метаданные файловой системы (проверка существования, удаление, размер, создание каталогов).
	 *          Не выполняет ввод-вывод байт (это задача ReadOnlyFile и ReadWriteFile).
	 */
	class FileSystemBase
	{
		friend class engine::Engine;
		friend class ReadOnlyFile;
		friend class ReadWriteFile;

	public:
		explicit FileSystemBase(std::shared_ptr<NativeAppData> nativeData = nullptr);
		virtual ~FileSystemBase() = default;

		[[nodiscard]] virtual bool FileExists(eFileLocation location, const std::filesystem::path& relativePath) const noexcept;
		[[nodiscard]] virtual bool DirectoryExists(eFileLocation location, const std::filesystem::path& relativePath) const noexcept;
		[[nodiscard]] virtual std::expected<std::uintmax_t, std::string> GetFileSize(eFileLocation location, const std::filesystem::path& relativePath) const noexcept;
		[[nodiscard]] virtual std::expected<void, std::string> DeleteFile(eFileLocation location, const std::filesystem::path& relativePath) const noexcept;
		[[nodiscard]] virtual std::expected<void, std::string> CreateDirectories(eFileLocation location, const std::filesystem::path& relativePath) const noexcept;

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

		[[nodiscard]] std::shared_ptr<NativeAppData> GetNativeData() const noexcept { return m_NativeData; }

		[[nodiscard]] std::expected<void, std::string> InitializeUserData(std::string_view companyName, std::string_view appName) noexcept;

	protected:
		std::shared_ptr<NativeAppData> m_NativeData;
		std::shared_ptr<Path>          m_Path;
	};
}
