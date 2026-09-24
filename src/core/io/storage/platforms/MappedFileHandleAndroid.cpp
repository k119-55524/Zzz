#include "MappedFileHandle.h"

namespace zzz::core
{
	struct MappedFileHandle::Impl
	{
		std::size_t size = 0;
		const std::byte* data = nullptr;
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
		(void)physicalPath;
		return std::unexpected("MappedFileHandle для Android пока в разработке (отложено в TODO).");
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
