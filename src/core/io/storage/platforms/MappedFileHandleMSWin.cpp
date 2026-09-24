#include "MappedFileHandle.h"
#include "core/headers/platforms/MSWin.h"
#include "core/utils/SafeMath.h"

#include <format>

namespace zzz::core
{
	struct MappedFileHandle::Impl
	{
		std::size_t size = 0;
		const std::byte* data = nullptr;
		HANDLE file = INVALID_HANDLE_VALUE;
		HANDLE mapping = nullptr;

		~Impl()
		{
			if (data)
				UnmapViewOfFile(data);
			if (mapping)
				CloseHandle(mapping);
			if (file != INVALID_HANDLE_VALUE)
				CloseHandle(file);
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
		const std::filesystem::path& physicalPath)
	{
		auto impl = std::make_unique<Impl>();

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
