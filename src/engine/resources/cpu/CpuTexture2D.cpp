#include "core/utils/MemoryUtils.h"
#include "CpuTexture2D.h"

using namespace zzz::core;

namespace zzz::engine
{
	CpuTexture2D::CpuTexture2D(
		const Guid& guid,
		std::string name,
		uint32_t width,
		uint32_t height,
		std::vector<std::byte> pixelData)
		: ResourceBase(guid, eResourceType::Texture2D, std::move(name))
		, m_Width(width)
		, m_Height(height)
		, m_PixelData(std::move(pixelData))
	{
	}

	std::expected<std::shared_ptr<CpuTexture2D>, std::string> CpuTexture2D::CreateFromMemory(
		const PackageEntry& entry,
		std::span<const std::byte> /*bytes*/)
	{
		return safe_make_shared<CpuTexture2D>(entry.GetGuid(), std::string(entry.GetName()));
	}
}
