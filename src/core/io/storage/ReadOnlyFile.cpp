#include "ReadOnlyFile.h"
#include "platforms/MappedFileHandle.h"
#include "core/utils/SafeMath.h"

namespace zzz::core
{
	struct ReadOnlyFile::Impl
	{
		MappedFileHandle handle;
	};

	ReadOnlyFile::ReadOnlyFile() noexcept = default;

	ReadOnlyFile::ReadOnlyFile(std::unique_ptr<Impl> impl, std::filesystem::path path) noexcept
		: m_Impl(std::move(impl)), m_Path(std::move(path))
	{
	}

	ReadOnlyFile::ReadOnlyFile(const std::filesystem::path& physicalPath)
		: m_Path(physicalPath)
	{
		auto handleRes = MappedFileHandle::Open(physicalPath);
		if (!handleRes)
		{
			m_Error = handleRes.error();
			return;
		}

		auto impl = std::make_unique<Impl>();
		impl->handle = std::move(*handleRes);
		m_Impl = std::move(impl);
	}

	ReadOnlyFile::~ReadOnlyFile() = default;
	ReadOnlyFile::ReadOnlyFile(ReadOnlyFile&&) noexcept = default;
	ReadOnlyFile& ReadOnlyFile::operator=(ReadOnlyFile&&) noexcept = default;

	std::expected<ReadOnlyFile, std::string> ReadOnlyFile::Open(
		const std::filesystem::path& physicalPath)
	{
		auto handleRes = MappedFileHandle::Open(physicalPath);
		if (!handleRes)
			return std::unexpected(handleRes.error());

		auto impl = std::make_unique<Impl>();
		impl->handle = std::move(*handleRes);
		return ReadOnlyFile(std::move(impl), physicalPath);
	}

	const std::filesystem::path& ReadOnlyFile::GetPath() const noexcept
	{
		return m_Path;
	}

	std::span<const std::byte> ReadOnlyFile::GetSpan() const noexcept
	{
		if (!m_Impl || !m_Impl->handle.IsValid())
			return {};
		return { m_Impl->handle.GetData(), m_Impl->handle.GetSize() };
	}

	std::span<const std::byte> ReadOnlyFile::Subspan(std::size_t offset, std::size_t size) const noexcept
	{
		if (!m_Impl || !m_Impl->handle.IsValid())
			return {};
		const auto totalSize = m_Impl->handle.GetSize();
		if (offset > totalSize || size > totalSize - offset)
			return {};
		return GetSpan().subspan(offset, size);
	}

	std::size_t ReadOnlyFile::GetSize() const noexcept
	{
		return (m_Impl && m_Impl->handle.IsValid()) ? m_Impl->handle.GetSize() : 0;
	}

	bool ReadOnlyFile::IsValid() const noexcept
	{
		return m_Impl && m_Impl->handle.IsValid();
	}

	const std::string& ReadOnlyFile::GetError() const noexcept
	{
		return m_Error;
	}

	void ReadOnlyFile::Close() noexcept
	{
		if (m_Impl)
		{
			m_Impl->handle.Close();
			m_Impl.reset();
		}
		m_Path.clear();
		m_Error.clear();
	}
}
