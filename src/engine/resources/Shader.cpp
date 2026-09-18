#include "Shader.h"

using namespace zzz::core;

namespace zzz::engine
{
	Shader::Shader(const Guid& guid, std::string name)
		: ResourceBase(guid, eResourceType::Shader, std::move(name))
	{
	}
}
