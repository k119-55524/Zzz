#include "core/utils/Ensure.h"
#include "core/utils/MemoryUtils.h"
#include "GpuShader.h"

using namespace zzz::core;

namespace zzz::engine
{
	GpuShader::GpuShader(
		const Guid& guid,
		std::string name,
		ResourceRef<CpuShader> cpuShader)
		: ResourceBase(guid, eResourceType::Shader, std::move(name))
		, m_CpuShader(std::move(cpuShader))
	{
	}

	std::shared_ptr<GpuShader> GpuShader::CreateFromCpu(ResourceRef<CpuShader> cpuShader)
	{
		ensure(cpuShader != nullptr, "GpuShader::CreateFromCpu: cpuShader не должен быть null");
		const auto& guid = cpuShader->GetGuid();
		std::string name(cpuShader->GetName());
		return safe_make_shared<GpuShader>(guid, std::move(name), std::move(cpuShader));
	}
}
