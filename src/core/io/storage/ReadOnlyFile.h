#pragma once

#include <cstddef>
#include <expected>
#include <filesystem>
#include <memory>
#include <span>
#include <string>

#include "core/utils/Defines.h"

namespace zzz::core
{
	/**
	 * @brief Неизменяемое отображение физического файла в память процесса (Zero-Copy Read-Only).
	 * @details Реализует строгое чтение через Memory Mapping (Win32 MapViewOfFile, POSIX mmap).
	 */
	class ReadOnlyFile final
	{
	public:
#if defined(Z_DESKTOP) || defined(Z_IOS)
		static constexpr bool c_IsSupported = true;
#else
		static constexpr bool c_IsSupported = false;
#endif

		ReadOnlyFile() noexcept;
		explicit ReadOnlyFile(const std::filesystem::path& physicalPath);
		~ReadOnlyFile();

		ReadOnlyFile(const ReadOnlyFile&) = delete;
		ReadOnlyFile& operator=(const ReadOnlyFile&) = delete;
		ReadOnlyFile(ReadOnlyFile&&) noexcept;
		ReadOnlyFile& operator=(ReadOnlyFile&&) noexcept;

		[[nodiscard]] static std::expected<ReadOnlyFile, std::string> Open(
			const std::filesystem::path& physicalPath);

		[[nodiscard]] const std::filesystem::path& GetPath() const noexcept;
		[[nodiscard]] std::span<const std::byte> GetSpan() const noexcept;
		[[nodiscard]] std::span<const std::byte> Subspan(std::size_t offset, std::size_t size) const noexcept;
		[[nodiscard]] std::size_t GetSize() const noexcept;
		[[nodiscard]] bool IsValid() const noexcept;
		[[nodiscard]] const std::string& GetError() const noexcept;
		void Close() noexcept;

	private:
		struct Impl;
		explicit ReadOnlyFile(std::unique_ptr<Impl> impl, std::filesystem::path path = {}) noexcept;

		std::unique_ptr<Impl> m_Impl;
		std::filesystem::path m_Path;
		std::string m_Error;
	};
}
