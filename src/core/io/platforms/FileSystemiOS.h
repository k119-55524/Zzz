#pragma once

#include "core/io/FileSystemBase.h"

namespace zzz::core
{
	/**
	 * @class FileSystemiOS
	 * @brief Специализация файловой системы для платформы iOS.
	 *
	 * @details eFileLocation::App читается из ресурсов NSBundle (реализация запланирована на КП-5).
	 *          Запись в eFileLocation::App запрещена.
	 *          Пользовательские данные пишутся в песочницу приложения (NSApplicationSupportDirectory) через FileSystemBase.
	 */
	class FileSystemiOS final : public FileSystemBase
	{
	public:
		using FileSystemBase::FileSystemBase;
		~FileSystemiOS() = default;

		[[nodiscard]] bool FileExists(eFileLocation location, std::string_view relativePath) const noexcept;

		[[nodiscard]] std::expected<std::vector<std::byte>, std::string> ReadBytes(
			eFileLocation location, std::string_view relativePath, std::size_t offset, std::size_t size) const noexcept;

		[[nodiscard]] std::expected<std::vector<std::byte>, std::string> ReadAllBytes(
			eFileLocation location, std::string_view relativePath) const noexcept;

		std::expected<void, std::string> WriteAllBytes(
			eFileLocation location, std::string_view relativePath, std::span<const std::byte> bytes) noexcept;
	};
}
