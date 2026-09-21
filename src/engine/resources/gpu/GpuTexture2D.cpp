#include "core/utils/Ensure.h"
#include "core/utils/MemoryUtils.h"
#include "GpuTexture2D.h"

using namespace zzz::core;

namespace zzz::engine
{
	GpuTexture2D::GpuTexture2D(
		const Guid& guid,
		std::string name,
		ResourceRef<CpuTexture2D> cpuTexture)
		: ResourceBase(guid, eResourceType::Texture2D, std::move(name))
		, m_CpuTexture(std::move(cpuTexture))
	{
	}

	std::shared_ptr<GpuTexture2D> GpuTexture2D::CreateFromCpu(ResourceRef<CpuTexture2D> cpuTexture)
	{
		ensure(cpuTexture != nullptr, "GpuTexture2D::CreateFromCpu: cpuTexture не должен быть null");
		const auto& guid = cpuTexture->GetGuid();
		std::string name(cpuTexture->GetName());
		return safe_make_shared<GpuTexture2D>(guid, std::move(name), std::move(cpuTexture));
	}
}
