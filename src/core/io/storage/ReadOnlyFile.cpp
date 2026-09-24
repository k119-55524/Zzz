
#include "platforms/MappedFileHandle.h"

#include "ReadOnlyFile.h"

namespace zzz::core
{
	struct ReadOnlyFile::Impl
	{
		MappedFileHandle handle;
	};

	ReadOnlyFile::ReadOnlyFile(const std::filesystem::path& physicalPath)
	{
		auto handleRes = MappedFileHandle::Open(physicalPath);
		if (!handleRes)
			THROW_RUNTIME("Не удалось отобразить файл '{}' в память: {}", physicalPath.string(), handleRes.error());

		auto impl = std::make_unique<Impl>();
		impl->handle = std::move(*handleRes);
		m_Impl = std::move(impl);
	}

	ReadOnlyFile::~ReadOnlyFile() = default;

	std::span<const std::byte> ReadOnlyFile::GetSpan() const noexcept
	{
		return { m_Impl->handle.GetData(), m_Impl->handle.GetSize() };
	}

	std::span<const std::byte> ReadOnlyFile::Subspan(std::size_t offset, std::size_t size) const noexcept
	{
		const auto totalSize = m_Impl->handle.GetSize();
		if (offset > totalSize || size > totalSize - offset)
			return {};

		return GetSpan().subspan(offset, size);
	}
}
