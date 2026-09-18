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
}
