#pragma once

#include "engine/gapi/vulkan/VulkanAPI.h"

#if defined(Z_VULKAN)

namespace zzz::engine
{
	constexpr uint32_t FRAMES_IN_FLIGHT = 3;

	class Swapchain_VK final
	{
		Z_NO_COPY_MOVE(Swapchain_VK);

	public:
		Swapchain_VK();
		~Swapchain_VK();

		std::expected<VkExtent2D, std::string> Initialize(std::shared_ptr<zzz::engine::VulkanAPI> gapi, VkSurfaceKHR surface);

	private:
		VkSurfaceFormat2KHR SelectSwapSurfaceFormat(const std::vector<VkSurfaceFormat2KHR>& availableFormats) const;
		VkPresentModeKHR SelectSwapPresentMode(bool vSync, const std::vector<VkPresentModeKHR>& availablePresentModes);
		void CmdTransitionSwapchainLayout(VkCommandBuffer cmd, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout);

		struct Image
		{
			VkImage		image{ VK_NULL_HANDLE };
			VkImageView	imageView{ VK_NULL_HANDLE };
		};

		struct FrameResources
		{
			VkSemaphore imageAvailableSemaphore{ VK_NULL_HANDLE };
			VkSemaphore renderFinishedSemaphore{ VK_NULL_HANDLE };
		};

		VkDevice m_Device{ VK_NULL_HANDLE };
		VkSwapchainKHR m_SwapChain{ VK_NULL_HANDLE };
		VkFormat m_ImageFormat{};

		std::vector<Image> m_NextImages;
		std::vector<FrameResources> m_FrameResources;

		uint32_t m_MaxFramesInFlight = FRAMES_IN_FLIGHT;
	};
}

#endif // Z_VULKAN
