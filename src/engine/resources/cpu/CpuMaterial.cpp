#include "CpuMaterial.h"

using namespace zzz::core;

namespace zzz::engine
{
	CpuMaterial::CpuMaterial(const Guid& guid, std::string name, Guid shaderGuid)
		: ResourceBase(guid, eResourceType::Material, std::move(name))
		, m_ShaderGuid(shaderGuid)
	{
	}
}
