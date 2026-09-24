#include "ReadOnlyFile.h"
#include "platforms/MappedFileHandle.h"
#include "FileSystemBase.h"
#include "core/utils/SafeMath.h"

namespace zzz::core
{
	struct ReadOnlyFile::Impl
	{
		MappedFileHandle handle;
	};

	ReadOnlyFile::ReadOnlyFile() noexcept = default;

	ReadOnlyFile::ReadOnlyFile(std::unique_ptr<Impl> impl) noexcept
		: m_Impl(std::move(impl))
	{
	}

	ReadOnlyFile::ReadOnlyFile(const std::filesystem::path& physicalPath)
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

	ReadOnlyFile::ReadOnlyFile(
		const FileSystemBase& fileSystem,
		eFileLocation location,
		const std::filesystem::path& relativePath)
	{
		auto pathRes = fileSystem.ResolvePhysicalPath(location, relativePath);
		if (!pathRes)
		{
			m_Error = pathRes.error();
			return;
		}

		auto handleRes = MappedFileHandle::Open(*pathRes);
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
		return ReadOnlyFile(std::move(impl));
	}

	std::expected<ReadOnlyFile, std::string> ReadOnlyFile::Open(
		const FileSystemBase& fileSystem,
		eFileLocation location,
		const std::filesystem::path& relativePath)
	{
		auto pathRes = fileSystem.ResolvePhysicalPath(location, relativePath);
		if (!pathRes)
			return std::unexpected(pathRes.error());

		return Open(*pathRes);
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
	}
}
