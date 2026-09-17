#include "Material.h"

using namespace zzz::core;

namespace zzz::engine
{
	Material::Material(const Guid& guid, std::string name)
		: ResourceBase(guid, eResourceType::Material, std::move(name))
	{
	}
}
