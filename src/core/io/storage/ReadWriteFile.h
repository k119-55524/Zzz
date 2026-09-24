#pragma once

#include <cstddef>
#include <expected>
#include <filesystem>
#include <fstream>
#include <memory>
#include <mutex>
#include <span>
#include <string>
#include <vector>

#include "core/enums/eFileLocation.h"

namespace zzz::core
{
	class FileSystemBase;

	enum class eFileAccessMode
	{
		Read,        // Только чтение существующего файла
		Write,       // Перезапись (создаёт новый или усекает существующий до 0)
		ReadWrite,   // Чтение и запись (создаёт, если файла нет)
		Append       // Дозапись в конец (для логов и потоков)
	};

	/**
	 * @brief Изменяемый файл с потокобезопасными операциями чтения и записи.
	 * @details Все операции синхронизированы внутренним мьютексом.
	 */
	class ReadWriteFile final
	{
	public:
		ReadWriteFile() noexcept = default;

		explicit ReadWriteFile(
			const std::filesystem::path& physicalPath,
			eFileAccessMode mode = eFileAccessMode::ReadWrite);

		ReadWriteFile(
			const FileSystemBase& fileSystem,
			eFileLocation location,
			const std::filesystem::path& relativePath,
			eFileAccessMode mode = eFileAccessMode::ReadWrite);

		~ReadWriteFile();

		ReadWriteFile(const ReadWriteFile&) = delete;
		ReadWriteFile& operator=(const ReadWriteFile&) = delete;

		ReadWriteFile(ReadWriteFile&& other) noexcept;
		ReadWriteFile& operator=(ReadWriteFile&& other) noexcept;

		[[nodiscard]] static std::expected<ReadWriteFile, std::string> Open(
			const std::filesystem::path& physicalPath,
			eFileAccessMode mode = eFileAccessMode::ReadWrite);

		[[nodiscard]] static std::expected<ReadWriteFile, std::string> Open(
			const FileSystemBase& fileSystem,
			eFileLocation location,
			const std::filesystem::path& relativePath,
			eFileAccessMode mode = eFileAccessMode::ReadWrite);

		[[nodiscard]] bool IsValid() const noexcept;
		[[nodiscard]] const std::string& GetError() const noexcept;
		[[nodiscard]] std::size_t GetSize() const noexcept;

		[[nodiscard]] std::expected<std::vector<std::byte>, std::string> ReadAll() noexcept;
		std::expected<void, std::string> WriteAll(std::span<const std::byte> bytes) noexcept;

		[[nodiscard]] std::expected<std::size_t, std::string> Read(std::span<std::byte> destination) noexcept;
		std::expected<void, std::string> Write(std::span<const std::byte> bytes) noexcept;
		std::expected<void, std::string> Flush() noexcept;
		void Close() noexcept;

	private:
		mutable std::mutex    m_Mutex;
		std::fstream          m_Stream;
		std::filesystem::path m_Path;
		eFileAccessMode       m_Mode = eFileAccessMode::ReadWrite;
		std::string           m_Error;
		bool                  m_IsValid = false;

		void OpenInternal(const std::filesystem::path& physicalPath, eFileAccessMode mode);
	};
}
