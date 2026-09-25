
#include <format>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "core/utils/SafeMath.h"
#include "core/utils/Macroses.h"

#include "MappedFileHandle.h"

namespace zzz::core
{
	struct MappedFileHandle::Impl
	{
		std::size_t size = 0;
		const std::byte* data = nullptr;
		int file = -1;

		~Impl()
		{
			if (data)
				munmap(const_cast<std::byte*>(data), size);
			if (file >= 0)
				close(file);
		}
	};

	MappedFileHandle::MappedFileHandle() noexcept = default;

	MappedFileHandle::MappedFileHandle(std::unique_ptr<Impl> impl) noexcept
		: m_Impl(std::move(impl))
	{
	}

	MappedFileHandle::~MappedFileHandle() = default;
	MappedFileHandle::MappedFileHandle(MappedFileHandle&& other) noexcept = default;
	MappedFileHandle& MappedFileHandle::operator=(MappedFileHandle&& other) noexcept = default;

	std::expected<MappedFileHandle, std::string> MappedFileHandle::Open(
		const std::filesystem::path& physicalPath,
		[[maybe_unused]] NativeAppData* nativeData)
	{
		auto impl = std::make_unique<Impl>();

		impl->file = open(physicalPath.c_str(), O_RDONLY | O_CLOEXEC);
		if (impl->file < 0)
		{
			const int error = errno;
			return UNEXPECTED(
				"Не удалось открыть файл '{}' для отображения в память: {}.",
				physicalPath.string(), std::strerror(error));
		}

		struct stat fileStat{};
		if (fstat(impl->file, &fileStat) != 0)
		{
			const int error = errno;
			return UNEXPECTED(
				"Не удалось получить размер файла '{}': {}.",
				physicalPath.string(), std::strerror(error));
		}
		if (!S_ISREG(fileStat.st_mode))
			return UNEXPECTED("Путь '{}' не является обычным файлом.", physicalPath.string());
		if (fileStat.st_size < 0)
			return UNEXPECTED("Файл '{}' имеет некорректный отрицательный размер.", physicalPath.string());

		const auto mappedSize = NarrowTo<std::size_t>(static_cast<std::uint64_t>(fileStat.st_size));
		if (!mappedSize)
			return UNEXPECTED("Размер файла '{}' не представим типом std::size_t.", physicalPath.string());

		impl->size = *mappedSize;
		if (impl->size > 0)
		{
			void* mapped = mmap(nullptr, impl->size, PROT_READ, MAP_PRIVATE, impl->file, 0);
			if (mapped == MAP_FAILED)
			{
				const int error = errno;
				return UNEXPECTED(
					"Не удалось отобразить файл '{}' в память: {}.",
					physicalPath.string(), std::strerror(error));
			}
			impl->data = static_cast<const std::byte*>(mapped);
		}

		return MappedFileHandle(std::move(impl));
	}

	const std::byte* MappedFileHandle::GetData() const noexcept
	{
		return m_Impl ? m_Impl->data : nullptr;
	}

	std::size_t MappedFileHandle::GetSize() const noexcept
	{
		return m_Impl ? m_Impl->size : 0;
	}

	bool MappedFileHandle::IsValid() const noexcept
	{
		return m_Impl != nullptr;
	}

	void MappedFileHandle::Close() noexcept
	{
		m_Impl.reset();
	}
}
