
#include <format>

#include "core/utils/SafeMath.h"
#include "core/io/FileSystemBase.h"

#if defined(Z_WINDOWS)
#include "core/headers/platforms/MSWin.h"
#elif defined(Z_DESKTOP) || defined(Z_IOS)
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#include "MemoryMappedFile.h"

namespace zzz::core
{
	struct MemoryMappedFile::Impl
	{
		std::size_t size = 0;
		const std::byte* data = nullptr;

#if defined(Z_WINDOWS)
		HANDLE file = INVALID_HANDLE_VALUE;
		HANDLE mapping = nullptr;
#elif defined(Z_DESKTOP) || defined(Z_IOS)
		int file = -1;
#endif

		~Impl()
		{
#if defined(Z_WINDOWS)
			if (data)
				UnmapViewOfFile(data);
			if (mapping)
				CloseHandle(mapping);
			if (file != INVALID_HANDLE_VALUE)
				CloseHandle(file);
#elif defined(Z_DESKTOP) || defined(Z_IOS)
			if (data)
				munmap(const_cast<std::byte*>(data), size);
			if (file >= 0)
				close(file);
#endif
		}
	};

	MemoryMappedFile::MemoryMappedFile() noexcept = default;

	MemoryMappedFile::MemoryMappedFile(std::unique_ptr<Impl> impl) noexcept
		: m_Impl(std::move(impl))
	{
	}

	MemoryMappedFile::~MemoryMappedFile() = default;
	MemoryMappedFile::MemoryMappedFile(MemoryMappedFile&&) noexcept = default;
	MemoryMappedFile& MemoryMappedFile::operator=(MemoryMappedFile&&) noexcept = default;

	std::expected<MemoryMappedFile, std::string> MemoryMappedFile::Open(
		const FileSystemBase& fileSystem,
		eFileLocation location,
		const std::filesystem::path& relativePath)
	{
#if defined(Z_DESKTOP) || defined(Z_IOS)
		auto physicalPathRes = fileSystem.ResolvePhysicalPath(location, relativePath);
		if (!physicalPathRes)
			return std::unexpected(std::move(physicalPathRes.error()));

		const auto& physicalPath = *physicalPathRes;
		auto impl = std::make_unique<Impl>();

#if defined(Z_WINDOWS)
		impl->file = CreateFileW(
			physicalPath.c_str(),
			GENERIC_READ,
			FILE_SHARE_READ,
			nullptr,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS,
			nullptr);
		if (impl->file == INVALID_HANDLE_VALUE)
		{
			return std::unexpected(std::format(
				"Не удалось открыть файл '{}' для отображения в память (Win32 error: {}).",
				physicalPath.string(), GetLastError()));
		}

		LARGE_INTEGER fileSize{};
		if (!GetFileSizeEx(impl->file, &fileSize) || fileSize.QuadPart < 0)
		{
			return std::unexpected(std::format(
				"Не удалось получить размер файла '{}' (Win32 error: {}).",
				physicalPath.string(), GetLastError()));
		}

		const auto mappedSize = NarrowTo<std::size_t>(static_cast<std::uint64_t>(fileSize.QuadPart));
		if (!mappedSize)
			return std::unexpected(std::format("Размер файла '{}' не представим типом std::size_t.", physicalPath.string()));

		impl->size = *mappedSize;
		if (impl->size > 0)
		{
			impl->mapping = CreateFileMappingW(impl->file, nullptr, PAGE_READONLY, 0, 0, nullptr);
			if (!impl->mapping)
			{
				return std::unexpected(std::format(
					"Не удалось создать отображение файла '{}' (Win32 error: {}).",
					physicalPath.string(), GetLastError()));
			}

			impl->data = static_cast<const std::byte*>(MapViewOfFile(impl->mapping, FILE_MAP_READ, 0, 0, 0));
			if (!impl->data)
			{
				return std::unexpected(std::format(
					"Не удалось отобразить файл '{}' в память (Win32 error: {}).",
					physicalPath.string(), GetLastError()));
			}
		}
#else
		impl->file = open(physicalPath.c_str(), O_RDONLY);
		if (impl->file < 0)
		{
			return std::unexpected(std::format(
				"Не удалось открыть файл '{}' для отображения в память: {}.",
				physicalPath.string(), std::strerror(errno)));
		}

		struct stat fileStat{};
		if (fstat(impl->file, &fileStat) != 0 || fileStat.st_size < 0)
		{
			return std::unexpected(std::format(
				"Не удалось получить размер файла '{}': {}.",
				physicalPath.string(), std::strerror(errno)));
		}

		const auto mappedSize = NarrowTo<std::size_t>(static_cast<std::uint64_t>(fileStat.st_size));
		if (!mappedSize)
			return std::unexpected(std::format("Размер файла '{}' не представим типом std::size_t.", physicalPath.string()));

		impl->size = *mappedSize;
		if (impl->size > 0)
		{
			void* mapped = mmap(nullptr, impl->size, PROT_READ, MAP_SHARED, impl->file, 0);
			if (mapped == MAP_FAILED)
			{
				return std::unexpected(std::format(
					"Не удалось отобразить файл '{}' в память: {}.",
					physicalPath.string(), std::strerror(errno)));
			}
			impl->data = static_cast<const std::byte*>(mapped);
		}
#endif

		return MemoryMappedFile(std::move(impl));
#else
		(void)fileSystem;
		(void)location;
		(void)relativePath;
		return std::unexpected("MemoryMappedFile пока не поддерживается на мобильных платформах.");
#endif
	}

	std::span<const std::byte> MemoryMappedFile::GetSpan() const noexcept
	{
		if (!m_Impl)
			return {};
		return { m_Impl->data, m_Impl->size };
	}

	std::span<const std::byte> MemoryMappedFile::Subspan(std::size_t offset, std::size_t size) const noexcept
	{
		if (!m_Impl || offset > m_Impl->size || size > m_Impl->size - offset)
			return {};
		return GetSpan().subspan(offset, size);
	}

	std::size_t MemoryMappedFile::GetSize() const noexcept
	{
		return m_Impl ? m_Impl->size : 0;
	}

	bool MemoryMappedFile::IsValid() const noexcept
	{
		return m_Impl != nullptr;
	}
}
