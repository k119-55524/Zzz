#include "GpuMaterial_VK.h"
#include "core/utils/Ensure.h"
#include "core/utils/MemoryUtils.h"

using namespace zzz::core;

namespace zzz::engine
{
	GpuMaterial_VK::GpuMaterial_VK(
		const Guid& guid,
		std::string name,
		const Guid& shaderGuid)
		: ResourceBase(guid, eResourceType::Material, std::move(name))
		, m_ShaderGuid(shaderGuid)
	{
	}

	std::shared_ptr<GpuMaterial_VK> GpuMaterial_VK::CreateGpuResourceAndUploadFromCpu(ResourceRef<CpuMaterial> cpuMaterial)
	{
		ensure(cpuMaterial != nullptr, "GpuMaterial_VK::CreateGpuResourceAndUploadFromCpu: cpuMaterial не должен быть null");
		const auto& guid = cpuMaterial->GetGuid();
		std::string name(cpuMaterial->GetName());
		const Guid shaderGuid = cpuMaterial->GetShaderGuid();

		return safe_make_shared<GpuMaterial_VK>(guid, std::move(name), shaderGuid);
	}
}
