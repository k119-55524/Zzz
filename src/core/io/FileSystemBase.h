#pragma once

#include <span>
#include <memory>
#include <vector>
#include <string>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <filesystem>
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
	class MemoryMappedFile;

	/**
	 * @class FileSystemBase
	 * @brief Базовая реализация файловой системы для кроссплатформенного ввода-вывода.
	 *
	 * @details Реализует стандартный дисковый ввод-вывод через std::filesystem и std::fstream
	 *          для десктопных платформ (Windows, Linux, macOS), а также общую дисковую логику
	 *          пользовательских песочниц (User, Saves, Cache, Logs) для мобильных платформ.
	 */
	class FileSystemBase
	{
		friend class engine::Engine;
		friend class MemoryMappedFile;

	public:
		explicit FileSystemBase(std::shared_ptr<NativeAppData> nativeData = nullptr);
		~FileSystemBase() = default;

		[[nodiscard]] bool FileExists(eFileLocation location, const std::filesystem::path& relativePath) const noexcept;
		[[nodiscard]] std::expected<std::uintmax_t, std::string> GetFileSize(eFileLocation location, const std::filesystem::path& relativePath) const noexcept;
		[[nodiscard]] std::expected<std::vector<std::byte>, std::string> ReadBytes(eFileLocation location, const std::filesystem::path& relativePath, std::size_t offset, std::size_t size) const noexcept;
		[[nodiscard]] std::expected<std::vector<std::byte>, std::string> ReadAllBytes(eFileLocation location, const std::filesystem::path& relativePath) const noexcept;
		std::expected<void, std::string> WriteAllBytes(eFileLocation location, const std::filesystem::path& relativePath, std::span<const std::byte> bytes) noexcept;
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

	protected:
		[[nodiscard]] inline std::expected<std::filesystem::path, std::string> ResolvePhysicalPath(eFileLocation location, const std::filesystem::path& relativePath) const noexcept
		{
			auto dirRes = m_Path->GetDirectory(location);
			if (!dirRes)
				return std::unexpected(std::move(dirRes.error()));

			return *dirRes / relativePath;
		}

		std::shared_ptr<NativeAppData> m_NativeData;
		std::shared_ptr<Path>          m_Path;

	private:
		[[nodiscard]] inline std::expected<void, std::string> InitializeUserData(std::string_view companyName, std::string_view appName) noexcept
		{
			return m_Path->InitializeUserData(companyName, appName);
		}
	};
}
