#include "core/utils/Ensure.h"
#include "core/utils/MemoryUtils.h"
#include "GpuMaterial.h"

using namespace zzz::core;

namespace zzz::engine
{
	GpuMaterial::GpuMaterial(
		const Guid& guid,
		std::string name,
		ResourceRef<CpuMaterial> cpuMaterial)
		: ResourceBase(guid, eResourceType::Material, std::move(name))
		, m_CpuMaterial(std::move(cpuMaterial))
	{
	}

	std::shared_ptr<GpuMaterial> GpuMaterial::CreateFromCpu(ResourceRef<CpuMaterial> cpuMaterial)
	{
		ensure(cpuMaterial != nullptr, "GpuMaterial::CreateFromCpu: cpuMaterial не должен быть null");
		const auto& guid = cpuMaterial->GetGuid();
		std::string name(cpuMaterial->GetName());
		return safe_make_shared<GpuMaterial>(guid, std::move(name), std::move(cpuMaterial));
	}
}
