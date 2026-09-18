#include "GpuShader.h"

using namespace zzz::core;

namespace zzz::engine
{
	GpuShader::GpuShader(
		const Guid& guid,
		std::string name,
		std::shared_ptr<CpuShader> cpuShader)
		: ResourceBase(guid, eResourceType::Shader, std::move(name))
		, m_CpuShader(std::move(cpuShader))
	{
	}
}
