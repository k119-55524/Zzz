#include "CpuShader.h"

using namespace zzz::core;

namespace zzz::engine
{
	CpuShader::CpuShader(const Guid& guid, std::string name)
		: ResourceBase(guid, eResourceType::Shader, std::move(name))
	{
	}
}
