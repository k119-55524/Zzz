#include "GpuShader_VK.h"
#include "core/utils/Ensure.h"
#include "core/utils/MemoryUtils.h"

using namespace zzz::core;

namespace zzz::engine
{
	GpuShader_VK::GpuShader_VK(
		const Guid& guid,
		std::string name,
		ResourceRef<CpuShader> cpuShader)
		: ResourceBase(guid, eResourceType::Shader, std::move(name))
		, m_CpuShader(std::move(cpuShader))
	{
	}

	std::shared_ptr<GpuShader_VK> GpuShader_VK::CreateFromCpu(ResourceRef<CpuShader> cpuShader)
	{
		ensure(cpuShader != nullptr, "GpuShader_VK::CreateFromCpu: cpuShader не должен быть null");
		const auto& guid = cpuShader->GetGuid();
		std::string name(cpuShader->GetName());
		return safe_make_shared<GpuShader_VK>(guid, std::move(name), std::move(cpuShader));
	}
}
