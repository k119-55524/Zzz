#pragma once

#include "engine/gapi/vulkan/VulkanAPI.h"
#include "engine/gapi/clear_config/DepthBufferClearConfig.h"

#if defined(Z_VULKAN)

namespace zzz::engine
{
	using namespace zzz::core;

	class DepthBuffer_VK
	{
		Z_NO_COPY_MOVE(DepthBuffer_VK);

	public:
		DepthBuffer_VK(std::shared_ptr<VulkanAPI> gapi, const Size2D<>& size);
		~DepthBuffer_VK();

		void Release();
		void OnResize(const Size2D<>& size);

		void SetClearConfig(const DepthBufferClearConfig& config) noexcept { m_ClearConfig = config; }
		[[nodiscard]] const DepthBufferClearConfig& GetClearConfig() const noexcept { return m_ClearConfig; }

		[[nodiscard]] VkImage GetImage() const noexcept { return m_Image; }
		[[nodiscard]] VkImageView GetImageView() const noexcept { return m_ImageView; }
		[[nodiscard]] VkFormat GetFormat() const noexcept { return m_Format; }

	private:
		void CreateDepthResources(const Size2D<>& size);

		std::shared_ptr<VulkanAPI> m_GAPI;
		VkImage m_Image{ VK_NULL_HANDLE };
		VkDeviceMemory m_Memory{ VK_NULL_HANDLE };
		VkImageView m_ImageView{ VK_NULL_HANDLE };
		VkFormat m_Format{ c_DefaultDepthFormat };
		Size2D<> m_Size{};

		DepthBufferClearConfig m_ClearConfig;
	};
}

#endif // Z_VULKAN
