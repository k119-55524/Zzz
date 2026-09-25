#pragma once

#include <span>
#include <string>
#include <memory>
#include <cstddef>
#include <expected>
#include <filesystem>

#include "core/utils/Defines.h"
#include "core/utils/Macroses.h"
#include "core/utils/NativeAppData.h"

namespace zzz::core
{
	/**
	 * @brief Неизменяемое отображение физического файла в память процесса (Zero-Copy Read-Only).
	 * @details Реализует строгое чтение через Memory Mapping (Win32 MapViewOfFile, POSIX mmap)
	 *          либо через удерживаемый буфер AAsset на Android.
	 * @note Исходный файл должен оставаться неизменным до разрушения ReadOnlyFile.
	 */
	class ReadOnlyFile final
	{
		Z_NO_COPY_MOVE(ReadOnlyFile);

	public:
		explicit ReadOnlyFile(const std::filesystem::path& physicalPath, NativeAppData* nativeData = nullptr);
		~ReadOnlyFile();

		[[nodiscard]] std::expected<std::span<const std::byte>, std::string> Read(std::size_t offset = 0, std::size_t count = std::dynamic_extent) const;

	private:
		struct Impl;
		std::unique_ptr<Impl> m_Impl;
	};
}
