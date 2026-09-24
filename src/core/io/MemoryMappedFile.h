#pragma once

#include <cstddef>
#include <expected>
#include <filesystem>
#include <memory>
#include <span>
#include <string>

#include "core/enums/eFileLocation.h"
#include "core/utils/Defines.h"

namespace zzz::core
{
	class FileSystemBase;

	/**
	 * @brief Неизменяемое отображение физического файла в память процесса.
	 * @details На мобильных платформах пока не используется: архивы продолжают
	 *          читаться диапазонами через платформенный FileSystem.
	 */
	class MemoryMappedFile final
	{
	public:
#if defined(Z_DESKTOP) || defined(Z_IOS)
		static constexpr bool c_IsSupported = true;
#else
		static constexpr bool c_IsSupported = false;
#endif

		MemoryMappedFile() noexcept;
		~MemoryMappedFile();

		MemoryMappedFile(const MemoryMappedFile&) = delete;
		MemoryMappedFile& operator=(const MemoryMappedFile&) = delete;
		MemoryMappedFile(MemoryMappedFile&&) noexcept;
		MemoryMappedFile& operator=(MemoryMappedFile&&) noexcept;

		[[nodiscard]] static std::expected<MemoryMappedFile, std::string> Open(
			const FileSystemBase& fileSystem,
			eFileLocation location,
			const std::filesystem::path& relativePath);

		[[nodiscard]] std::span<const std::byte> GetSpan() const noexcept;
		[[nodiscard]] std::span<const std::byte> Subspan(std::size_t offset, std::size_t size) const noexcept;
		[[nodiscard]] std::size_t GetSize() const noexcept;
		[[nodiscard]] bool IsValid() const noexcept;

	private:
		struct Impl;
		explicit MemoryMappedFile(std::unique_ptr<Impl> impl) noexcept;

		std::unique_ptr<Impl> m_Impl;
	};
}
