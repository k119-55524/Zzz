#include "core/utils/Ensure.h"
#include "core/utils/MemoryUtils.h"
#include "GpuMaterial_VK.h"

using namespace zzz::core;

namespace zzz::engine
{
	GpuMaterial_VK::GpuMaterial_VK(
		const Guid& guid,
		std::string name,
		ResourceRef<CpuMaterial> cpuMaterial)
		: ResourceBase(guid, eResourceType::Material, std::move(name))
		, m_CpuMaterial(std::move(cpuMaterial))
	{
	}

	std::shared_ptr<GpuMaterial_VK> GpuMaterial_VK::CreateFromCpu(ResourceRef<CpuMaterial> cpuMaterial)
	{
		ensure(cpuMaterial != nullptr, "GpuMaterial_VK::CreateFromCpu: cpuMaterial не должен быть null");
		const auto& guid = cpuMaterial->GetGuid();
		std::string name(cpuMaterial->GetName());
		return safe_make_shared<GpuMaterial_VK>(guid, std::move(name), std::move(cpuMaterial));
	}
}
