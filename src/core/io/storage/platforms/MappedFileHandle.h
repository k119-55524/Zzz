#pragma once

#include <memory>
#include <string>
#include <cstddef>
#include <expected>
#include <filesystem>

#include "core/utils/Defines.h"
#include "core/utils/NativeAppData.h"

namespace zzz::core
{
	/**
	 * @brief Платформенно-изолированный RAII-хэндл отображения файла в виртуальную память.
	 * @details Реализация Impl выбирается на этапе сборки через CMake без макросов в бизнес-логике.
	 *          На Android удерживает AAsset и его буфер вместо системного mmap.
	 * @note Пока хэндл существует, источник нельзя изменять, удалять или заменять. Архивы
	 *       публикуются до запуска движка либо после его остановки; hot reload не поддерживается.
	 */
	class MappedFileHandle final
	{
	public:
		MappedFileHandle() noexcept;
		~MappedFileHandle();

		MappedFileHandle(const MappedFileHandle&) = delete;
		MappedFileHandle& operator=(const MappedFileHandle&) = delete;

		MappedFileHandle(MappedFileHandle&& other) noexcept;
		MappedFileHandle& operator=(MappedFileHandle&& other) noexcept;

		[[nodiscard]] static std::expected<MappedFileHandle, std::string> Open(
			const std::filesystem::path& physicalPath,
			NativeAppData* nativeData = nullptr);

		[[nodiscard]] const std::byte* GetData() const noexcept;
		[[nodiscard]] std::size_t GetSize() const noexcept;
		[[nodiscard]] bool IsValid() const noexcept;
		void Close() noexcept;

	private:
		struct Impl;
		explicit MappedFileHandle(std::unique_ptr<Impl> impl) noexcept;

		std::unique_ptr<Impl> m_Impl;
	};
}
