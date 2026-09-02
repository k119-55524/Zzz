#pragma once

#include <span>
#include <memory>
#include <vector>
#include <string>
#include <cstddef>
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
		friend class ::zzz::engine::Engine;

	public:
		explicit FileSystemBase(std::shared_ptr<NativeAppData> nativeData = nullptr);
		~FileSystemBase() = default;

		[[nodiscard]] bool FileExists(eFileLocation location, std::string_view relativePath) const noexcept;
		[[nodiscard]] std::expected<std::vector<std::byte>, std::string> ReadBytes(eFileLocation location, std::string_view relativePath, std::size_t offset, std::size_t size) const noexcept;
		[[nodiscard]] std::expected<std::vector<std::byte>, std::string> ReadAllBytes(eFileLocation location, std::string_view relativePath) const noexcept;
		std::expected<void, std::string> WriteAllBytes(eFileLocation location, std::string_view relativePath, std::span<const std::byte> bytes) noexcept;

	protected:
		[[nodiscard]] std::expected<std::filesystem::path, std::string> ResolvePhysicalPath(eFileLocation location, std::string_view relativePath) const noexcept;

		std::shared_ptr<NativeAppData> m_NativeData;
		std::shared_ptr<Path>          m_Path;

	private:
		[[nodiscard]] inline std::expected<void, std::string> InitializeUserData(std::string_view companyName, std::string_view appName) noexcept
		{
			return m_Path->InitializeUserData(companyName, appName);
		}
	};
}
