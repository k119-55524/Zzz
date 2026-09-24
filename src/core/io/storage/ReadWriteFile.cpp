#include "ReadWriteFile.h"
#include "FileSystemBase.h"
#include "core/utils/SafeMath.h"

#include <format>
#include <system_error>

namespace zzz::core
{
	namespace
	{
		std::ios::openmode ConvertAccessMode(eFileAccessMode mode) noexcept
		{
			switch (mode)
			{
			case eFileAccessMode::Read:
				return std::ios::in | std::ios::binary;
			case eFileAccessMode::Write:
				return std::ios::out | std::ios::binary | std::ios::trunc;
			case eFileAccessMode::ReadWrite:
				return std::ios::in | std::ios::out | std::ios::binary;
			case eFileAccessMode::Append:
				return std::ios::out | std::ios::binary | std::ios::app;
			}
			return std::ios::in | std::ios::out | std::ios::binary;
		}
	}

	ReadWriteFile::ReadWriteFile(
		const std::filesystem::path& physicalPath,
		eFileAccessMode mode)
	{
		OpenInternal(physicalPath, mode);
	}

	ReadWriteFile::ReadWriteFile(
		const FileSystemBase& fileSystem,
		eFileLocation location,
		const std::filesystem::path& relativePath,
		eFileAccessMode mode)
	{
		if ((mode == eFileAccessMode::Write || mode == eFileAccessMode::ReadWrite || mode == eFileAccessMode::Append)
			&& !FileSystemBase::IsLocationWritable(location))
		{
			m_Error = std::format("Попытка открыть на запись защищённую область: {}", ToString(location));
			m_IsValid = false;
			return;
		}

		auto pathRes = fileSystem.ResolvePhysicalPath(location, relativePath);
		if (!pathRes)
		{
			m_Error = pathRes.error();
			m_IsValid = false;
			return;
		}

		OpenInternal(*pathRes, mode);
	}

	ReadWriteFile::~ReadWriteFile()
	{
		Close();
	}

	ReadWriteFile::ReadWriteFile(ReadWriteFile&& other) noexcept
	{
		std::lock_guard<std::mutex> lock(other.m_Mutex);
		m_Stream = std::move(other.m_Stream);
		m_Path = std::move(other.m_Path);
		m_Mode = other.m_Mode;
		m_Error = std::move(other.m_Error);
		m_IsValid = other.m_IsValid;
		other.m_IsValid = false;
	}

	ReadWriteFile& ReadWriteFile::operator=(ReadWriteFile&& other) noexcept
	{
		if (this != &other)
		{
			std::scoped_lock lock(m_Mutex, other.m_Mutex);
			if (m_Stream.is_open())
				m_Stream.close();

			m_Stream = std::move(other.m_Stream);
			m_Path = std::move(other.m_Path);
			m_Mode = other.m_Mode;
			m_Error = std::move(other.m_Error);
			m_IsValid = other.m_IsValid;
			other.m_IsValid = false;
		}
		return *this;
	}

	void ReadWriteFile::OpenInternal(const std::filesystem::path& physicalPath, eFileAccessMode mode)
	{
		m_Path = physicalPath;
		m_Mode = mode;
		m_Error.clear();
		m_IsValid = false;

		std::error_code ec;
		if (mode == eFileAccessMode::Write || mode == eFileAccessMode::ReadWrite || mode == eFileAccessMode::Append)
		{
			const auto parentDir = physicalPath.parent_path();
			if (!parentDir.empty())
			{
				std::filesystem::create_directories(parentDir, ec);
				if (ec)
				{
					m_Error = std::format("Не удалось создать каталог '{}': {}", parentDir.string(), ec.message());
					return;
				}
			}

			// Если файл не существует при ReadWrite, сначала создаём его
			if (mode == eFileAccessMode::ReadWrite && !std::filesystem::exists(physicalPath, ec))
			{
				std::ofstream touch(physicalPath, std::ios::binary | std::ios::app);
			}
		}

		const auto openMode = ConvertAccessMode(mode);
		m_Stream.open(physicalPath, openMode);
		if (!m_Stream.is_open())
		{
			m_Error = std::format("Не удалось открыть файл '{}'", physicalPath.string());
			return;
		}

		m_IsValid = true;
	}

	std::expected<ReadWriteFile, std::string> ReadWriteFile::Open(
		const std::filesystem::path& physicalPath,
		eFileAccessMode mode)
	{
		ReadWriteFile file(physicalPath, mode);
		if (!file.IsValid())
			return std::unexpected(file.GetError());
		return file;
	}

	std::expected<ReadWriteFile, std::string> ReadWriteFile::Open(
		const FileSystemBase& fileSystem,
		eFileLocation location,
		const std::filesystem::path& relativePath,
		eFileAccessMode mode)
	{
		ReadWriteFile file(fileSystem, location, relativePath, mode);
		if (!file.IsValid())
			return std::unexpected(file.GetError());
		return file;
	}

	const std::filesystem::path& ReadWriteFile::GetPath() const noexcept
	{
		return m_Path;
	}

	bool ReadWriteFile::IsValid() const noexcept
	{
		std::lock_guard<std::mutex> lock(m_Mutex);
		return m_IsValid && m_Stream.is_open();
	}

	const std::string& ReadWriteFile::GetError() const noexcept
	{
		return m_Error;
	}

	std::size_t ReadWriteFile::GetSize() const noexcept
	{
		std::lock_guard<std::mutex> lock(m_Mutex);
		if (!m_IsValid || m_Path.empty())
			return 0;

		std::error_code ec;
		const auto sz = std::filesystem::file_size(m_Path, ec);
		if (ec)
			return 0;

		const auto narrowSz = NarrowTo<std::size_t>(sz);
		return narrowSz.value_or(0);
	}

	std::expected<std::vector<std::byte>, std::string> ReadWriteFile::ReadAll() noexcept
	{
		std::lock_guard<std::mutex> lock(m_Mutex);
		if (!m_IsValid || !m_Stream.is_open())
			return std::unexpected("Файл не открыт для чтения.");

		m_Stream.clear();
		m_Stream.seekg(0, std::ios::end);
		const auto streamEnd = m_Stream.tellg();
		if (streamEnd < 0)
			return std::unexpected(std::format("Не удалось определить размер файла '{}'", m_Path.string()));

		const auto size = NarrowTo<std::size_t>(static_cast<std::uint64_t>(streamEnd));
		if (!size)
			return std::unexpected(std::format("Размер файла '{}' не представим типом std::size_t", m_Path.string()));

		if (*size == 0)
			return std::vector<std::byte>{};

		m_Stream.seekg(0, std::ios::beg);
		std::vector<std::byte> buffer(*size);
		m_Stream.read(reinterpret_cast<char*>(buffer.data()), static_cast<std::streamsize>(*size));

		const auto readCount = static_cast<std::size_t>(m_Stream.gcount());
		if (readCount != *size)
		{
			return std::unexpected(std::format(
				"Прочитано {} из {} байт файла '{}'", readCount, *size, m_Path.string()));
		}

		return buffer;
	}

	std::expected<void, std::string> ReadWriteFile::WriteAll(std::span<const std::byte> bytes) noexcept
	{
		std::lock_guard<std::mutex> lock(m_Mutex);
		if (!m_IsValid)
			return std::unexpected("Файл не открыт для записи.");

		if (m_Stream.is_open())
			m_Stream.close();

		const auto openMode = ConvertAccessMode(m_Mode) | std::ios::trunc;
		m_Stream.open(m_Path, openMode);
		if (!m_Stream.is_open())
		{
			m_IsValid = false;
			return std::unexpected(std::format("Не удалось открыть файл для записи '{}'", m_Path.string()));
		}

		if (!bytes.empty())
		{
			m_Stream.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
			if (!m_Stream)
				return std::unexpected(std::format("Ошибка записи данных в файл '{}'", m_Path.string()));
		}

		m_Stream.flush();
		if (!m_Stream)
			return std::unexpected(std::format("Ошибка сброса буфера (flush) в файл '{}'", m_Path.string()));

		m_IsValid = true;
		return {};
	}

	std::expected<std::size_t, std::string> ReadWriteFile::Read(std::span<std::byte> destination) noexcept
	{
		std::lock_guard<std::mutex> lock(m_Mutex);
		if (!m_IsValid || !m_Stream.is_open())
			return std::unexpected("Файл не открыт для чтения.");

		if (destination.empty())
			return 0;

		m_Stream.read(reinterpret_cast<char*>(destination.data()), static_cast<std::streamsize>(destination.size()));
		const auto count = static_cast<std::size_t>(m_Stream.gcount());
		if (m_Stream.bad())
			return std::unexpected(std::format("Ошибка чтения из файла '{}'", m_Path.string()));

		return count;
	}

	std::expected<void, std::string> ReadWriteFile::Write(std::span<const std::byte> bytes) noexcept
	{
		std::lock_guard<std::mutex> lock(m_Mutex);
		if (!m_IsValid || !m_Stream.is_open())
			return std::unexpected("Файл не открыт для записи.");

		if (!bytes.empty())
		{
			m_Stream.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
			if (!m_Stream)
				return std::unexpected(std::format("Ошибка записи данных в файл '{}'", m_Path.string()));
		}

		return {};
	}

	std::expected<void, std::string> ReadWriteFile::Flush() noexcept
	{
		std::lock_guard<std::mutex> lock(m_Mutex);
		if (!m_IsValid || !m_Stream.is_open())
			return std::unexpected("Файл не открыт.");

		m_Stream.flush();
		if (!m_Stream)
			return std::unexpected(std::format("Ошибка сброса буфера в файл '{}'", m_Path.string()));

		return {};
	}

	void ReadWriteFile::Close() noexcept
	{
		std::lock_guard<std::mutex> lock(m_Mutex);
		if (m_Stream.is_open())
			m_Stream.close();
		m_IsValid = false;
	}
}
