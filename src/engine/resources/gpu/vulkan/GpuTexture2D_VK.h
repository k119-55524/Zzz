#pragma once

#include <string>
#include <memory>
#include <cstdint>
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

		GpuTexture2D_VK(const Guid& guid, std::string name, uint32_t width, uint32_t height);
		~GpuTexture2D_VK() override = default;

		[[nodiscard]] static std::shared_ptr<GpuTexture2D_VK> CreateGpuResourceAndUploadFromCpu(ResourceRef<CpuTexture2D> cpuTexture);

		[[nodiscard]] uint32_t GetWidth() const noexcept { return m_Width; }
		[[nodiscard]] uint32_t GetHeight() const noexcept { return m_Height; }

	private:
		uint32_t m_Width{ 0 };
		uint32_t m_Height{ 0 };
	};
}
