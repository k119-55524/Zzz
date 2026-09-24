#pragma once

#include <span>
#include <memory>
#include <cstddef>
#include <filesystem>

#include "core/utils/Macroses.h"

namespace zzz::core
{
	/**
	 * @brief Неизменяемое отображение физического файла в память процесса (Zero-Copy Read-Only).
	 * @details Реализует строгое чтение через Memory Mapping (Win32 MapViewOfFile, POSIX mmap).
	 */
	class ReadOnlyFile final
	{
	public:
		Z_NO_COPY_MOVE(ReadOnlyFile);

		explicit ReadOnlyFile(const std::filesystem::path& physicalPath);
		~ReadOnlyFile();

		[[nodiscard]] std::span<const std::byte> GetSpan() const noexcept;
		[[nodiscard]] std::span<const std::byte> Subspan(std::size_t offset, std::size_t size) const noexcept;

	private:
		struct Impl;
		std::unique_ptr<Impl> m_Impl;
	};
}
