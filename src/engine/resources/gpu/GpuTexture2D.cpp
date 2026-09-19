#include "GpuTexture2D.h"

using namespace zzz::core;

namespace zzz::engine
{
	GpuTexture2D::GpuTexture2D(
		const Guid& guid,
		std::string name,
		ResourceRef<CpuTexture2D> cpuTexture)
		: ResourceBase(guid, eResourceType::Texture2D, std::move(name))
		, m_CpuTexture(std::move(cpuTexture))
	{
	}
}
