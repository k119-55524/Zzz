
#include <format>

#include "core/utils/SafeMath.h"
#include "core/utils/Macroses.h"
#include "core/headers/platforms/MSWin.h"

#include "MappedFileHandle.h"

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
		const std::filesystem::path& physicalPath,
		[[maybe_unused]] NativeAppData* nativeData)
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
			const DWORD error = GetLastError();
			return UNEXPECTED("Не удалось открыть файл '{}' для отображения в память (Win32 error: {}).", physicalPath.string(), error);
		}

		BY_HANDLE_FILE_INFORMATION fileInfo{};
		if (!GetFileInformationByHandle(impl->file, &fileInfo))
		{
			const DWORD error = GetLastError();
			return UNEXPECTED("Не удалось получить сведения о файле '{}' (Win32 error: {}).", physicalPath.string(), error);
		}

		if ((fileInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0 || GetFileType(impl->file) != FILE_TYPE_DISK)
			return UNEXPECTED("Путь '{}' не является обычным файлом.", physicalPath.string());

		LARGE_INTEGER fileSize{};
		if (!GetFileSizeEx(impl->file, &fileSize))
		{
			const DWORD error = GetLastError();
			return UNEXPECTED("Не удалось получить размер файла '{}' (Win32 error: {}).", physicalPath.string(), error);
		}
		if (fileSize.QuadPart < 0)
			return UNEXPECTED("Файл '{}' имеет некорректный отрицательный размер.", physicalPath.string());

		const auto mappedSize = NarrowTo<std::size_t>(static_cast<std::uint64_t>(fileSize.QuadPart));
		if (!mappedSize)
			return UNEXPECTED("Размер файла '{}' не представим типом std::size_t.", physicalPath.string());

		impl->size = *mappedSize;
		if (impl->size > 0)
		{
			impl->mapping = CreateFileMappingW(impl->file, nullptr, PAGE_READONLY, 0, 0, nullptr);
			if (!impl->mapping)
			{
				const DWORD error = GetLastError();
				return UNEXPECTED("Не удалось создать отображение файла '{}' (Win32 error: {}).", physicalPath.string(), error);
			}

			impl->data = static_cast<const std::byte*>(MapViewOfFile(impl->mapping, FILE_MAP_READ, 0, 0, 0));
			if (!impl->data)
			{
				const DWORD error = GetLastError();
				return UNEXPECTED("Не удалось отобразить файл '{}' в память (Win32 error: {}).", physicalPath.string(), error);
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
