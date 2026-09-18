#include "Material.h"

using namespace zzz::core;

namespace zzz::engine
{
	Material::Material(const Guid& guid, std::string name, Guid shaderGuid)
		: ResourceBase(guid, eResourceType::Material, std::move(name)),
		  m_ShaderGuid(shaderGuid)
	{
	}
}
