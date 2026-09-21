#include "GpuTexture2D_VK.h"
#include "core/utils/Ensure.h"
#include "core/utils/MemoryUtils.h"

using namespace zzz::core;

namespace zzz::engine
{
	GpuTexture2D_VK::GpuTexture2D_VK(
		const Guid& guid,
		std::string name,
		uint32_t width,
		uint32_t height)
		: ResourceBase(guid, eResourceType::Texture2D, std::move(name))
		, m_Width(width)
		, m_Height(height)
	{
	}

	std::shared_ptr<GpuTexture2D_VK> GpuTexture2D_VK::CreateGpuResourceAndUploadFromCpu(ResourceRef<CpuTexture2D> cpuTexture)
	{
		ensure(cpuTexture != nullptr, "GpuTexture2D_VK::CreateGpuResourceAndUploadFromCpu: cpuTexture не должен быть null");
		const auto& guid = cpuTexture->GetGuid();
		std::string name(cpuTexture->GetName());
		const uint32_t width = cpuTexture->GetWidth();
		const uint32_t height = cpuTexture->GetHeight();

		return safe_make_shared<GpuTexture2D_VK>(guid, std::move(name), width, height);
	}
}
