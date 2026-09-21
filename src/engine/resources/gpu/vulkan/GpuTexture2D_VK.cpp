#include "core/utils/Ensure.h"
#include "core/utils/MemoryUtils.h"
#include "GpuTexture2D_VK.h"

using namespace zzz::core;

namespace zzz::engine
{
	GpuTexture2D_VK::GpuTexture2D_VK(
		const Guid& guid,
		std::string name,
		ResourceRef<CpuTexture2D> cpuTexture)
		: ResourceBase(guid, eResourceType::Texture2D, std::move(name))
		, m_CpuTexture(std::move(cpuTexture))
	{
	}

	std::shared_ptr<GpuTexture2D_VK> GpuTexture2D_VK::CreateFromCpu(ResourceRef<CpuTexture2D> cpuTexture)
	{
		ensure(cpuTexture != nullptr, "GpuTexture2D_VK::CreateFromCpu: cpuTexture не должен быть null");
		const auto& guid = cpuTexture->GetGuid();
		std::string name(cpuTexture->GetName());
		return safe_make_shared<GpuTexture2D_VK>(guid, std::move(name), std::move(cpuTexture));
	}
}
