
#include "platforms/MappedFileHandle.h"

#include "ReadOnlyFile.h"

namespace zzz::core
{
	struct ReadOnlyFile::Impl
	{
		std::filesystem::path path;
		MappedFileHandle handle;
	};

	ReadOnlyFile::ReadOnlyFile(const std::filesystem::path& physicalPath, NativeAppData* nativeData)
	{
		auto handleRes = MappedFileHandle::Open(physicalPath, nativeData);
		if (!handleRes)
			THROW_RUNTIME("Не удалось отобразить файл '{}' в память: {}", physicalPath.string(), handleRes.error());

		auto impl = std::make_unique<Impl>();
		impl->path = physicalPath;
		impl->handle = std::move(*handleRes);
		m_Impl = std::move(impl);
	}

	ReadOnlyFile::~ReadOnlyFile() = default;

	std::expected<std::span<const std::byte>, std::string> ReadOnlyFile::Read(std::size_t offset, std::size_t count) const
	{
		const auto totalSize = m_Impl->handle.GetSize();
		if (offset > totalSize)
			return UNEXPECTED("ReadOnlyFile::Read('{}'): смещение {} выходит за границы файла (размер: {})", m_Impl->path.string(), offset, totalSize);

		const auto actualSize = (count == std::dynamic_extent) ? (totalSize - offset) : count;
		if (actualSize > totalSize - offset)
			return UNEXPECTED("ReadOnlyFile::Read('{}'): диапазон [{}, {}) выходит за границы файла (размер: {})", m_Impl->path.string(), offset, offset + actualSize, totalSize);

		return std::span<const std::byte>{ m_Impl->handle.GetData() + offset, actualSize };
	}
}
