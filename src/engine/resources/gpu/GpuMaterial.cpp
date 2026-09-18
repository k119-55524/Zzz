#include "GpuMaterial.h"

using namespace zzz::core;

namespace zzz::engine
{
	GpuMaterial::GpuMaterial(
		const Guid& guid,
		std::string name,
		std::shared_ptr<CpuMaterial> cpuMaterial)
		: ResourceBase(guid, eResourceType::Material, std::move(name))
		, m_CpuMaterial(std::move(cpuMaterial))
	{
	}
}
