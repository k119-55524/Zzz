#include "GpuShader_VK.h"
#include "core/utils/Ensure.h"
#include "core/utils/MemoryUtils.h"

using namespace zzz::core;

namespace zzz::engine
{
	GpuShader_VK::GpuShader_VK(
		const Guid& guid,
		std::string name)
		: ResourceBase(guid, eEngineResourceType::Shader, std::move(name))
	{
	}

	std::shared_ptr<GpuShader_VK> GpuShader_VK::CreateGpuResourceAndUploadFromCpu(ResourceRef<CpuShader> cpuShader)
	{
		ensure(cpuShader != nullptr, "GpuShader_VK::CreateGpuResourceAndUploadFromCpu: cpuShader не должен быть null");
		const auto& guid = cpuShader->GetGuid();
		std::string name(cpuShader->GetName());

		return safe_make_shared<GpuShader_VK>(guid, std::move(name));
	}
}
