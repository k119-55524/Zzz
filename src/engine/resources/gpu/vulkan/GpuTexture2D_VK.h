#pragma once

#include <string>
#include <memory>

#include "engine/resources/ResourceRef.h"
#include "engine/resources/ResourceBase.h"
#include "engine/resources/cpu/CpuTexture2D.h"

using namespace zzz::core;

namespace zzz::engine
{
	/**
	 * @class GpuTexture2D_VK
	 * @brief Ресурс 2D-текстуры в видеопамяти (Vulkan: VkImage, VkImageView, VkSampler).
	 */
	class GpuTexture2D_VK final : public ResourceBase
	{
	public:
		using CpuSource = CpuTexture2D;

		GpuTexture2D_VK(const Guid& guid, std::string name, ResourceRef<CpuTexture2D> cpuTexture);
		~GpuTexture2D_VK() override = default;

		[[nodiscard]] static std::shared_ptr<GpuTexture2D_VK> CreateFromCpu(ResourceRef<CpuTexture2D> cpuTexture);

		[[nodiscard]] const ResourceRef<CpuTexture2D>& GetCpuTexture() const noexcept { return m_CpuTexture; }
		[[nodiscard]] uint32_t GetWidth() const noexcept { return m_CpuTexture ? m_CpuTexture->GetWidth() : 0; }
		[[nodiscard]] uint32_t GetHeight() const noexcept { return m_CpuTexture ? m_CpuTexture->GetHeight() : 0; }

	private:
		ResourceRef<CpuTexture2D> m_CpuTexture;
	};
}
