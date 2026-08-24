#include "Swapchain_VK.h"

#if defined(Z_VULKAN)

namespace zzz::engine
{
	constexpr std::array<VkFormat, 3> PREFERRED_FORMATS = {
		VK_FORMAT_B8G8R8A8_SRGB,
		VK_FORMAT_R8G8B8A8_SRGB,
		VK_FORMAT_A2B10G10R10_UNORM_PACK32
	};

	Swapchain_VK::Swapchain_VK() :
		m_NextImages(FRAMES_IN_FLIGHT),
		m_FrameResources(FRAMES_IN_FLIGHT)
	{}

	Swapchain_VK::~Swapchain_VK()
	{
		if (!m_Device)
			return;

		vkDeviceWaitIdle(m_Device);

		for (auto& image : m_NextImages)
		{
			if (image.imageView != VK_NULL_HANDLE)
				vkDestroyImageView(m_Device, image.imageView, nullptr);
		}

		if (m_SwapChain != VK_NULL_HANDLE)
			vkDestroySwapchainKHR(m_Device, m_SwapChain, nullptr);

		for (auto& frameRes : m_FrameResources)
		{
			if (frameRes.imageAvailableSemaphore)
				vkDestroySemaphore(m_Device, frameRes.imageAvailableSemaphore, nullptr);

			if (frameRes.renderFinishedSemaphore)
				vkDestroySemaphore(m_Device, frameRes.renderFinishedSemaphore, nullptr);
		}
	}

	std::expected<VkExtent2D, std::string> Swapchain_VK::Initialize(std::shared_ptr<zzz::engine::VulkanAPI> gapi, VkSurfaceKHR surface)
	{
		ensure(gapi, "VulkanAPI cannot be null.");
		VkPhysicalDevice physicalDevice = gapi->GetPhysicalDevice();
		ensure(physicalDevice, "Physical device cannot be null.");
		m_Device = gapi->GetDevice();
		ensure(m_Device, "Device cannot be null.");
		ensure(surface, "Surface cannot be null.");
		VkExtent2D winSize;

		const VkPhysicalDeviceSurfaceInfo2KHR surfaceInfo2
		{
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR,
			.surface = surface
		};

		VkSurfaceCapabilities2KHR capabilities2{ .sType = VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_2_KHR };
		VkResult vr = vkGetPhysicalDeviceSurfaceCapabilities2KHR(physicalDevice, &surfaceInfo2, &capabilities2);
		if (vr != VK_SUCCESS)
			return std::unexpected(std::format("Failed to vkGetPhysicalDeviceSurfaceCapabilities2KHR: {}", static_cast<int>(vr)));

		uint32_t formatCount = 0;
		vr = vkGetPhysicalDeviceSurfaceFormats2KHR(physicalDevice, &surfaceInfo2, &formatCount, nullptr);
		if (vr != VK_SUCCESS)
			return std::unexpected(std::format("Failed to vkGetPhysicalDeviceSurfaceFormats2KHR: {}", static_cast<int>(vr)));

		std::vector<VkSurfaceFormat2KHR> formats(formatCount, { .sType = VK_STRUCTURE_TYPE_SURFACE_FORMAT_2_KHR });
		vr = vkGetPhysicalDeviceSurfaceFormats2KHR(physicalDevice, &surfaceInfo2, &formatCount, formats.data());
		if (vr != VK_SUCCESS)
			return std::unexpected(std::format("Failed to vkGetPhysicalDeviceSurfaceFormats2KHR: {}", static_cast<int>(vr)));

		uint32_t presentModeCount = 0;
		vr = vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);
		if (vr != VK_SUCCESS)
			return std::unexpected(std::format("Failed to vkGetPhysicalDeviceSurfacePresentModesKHR: {}", static_cast<int>(vr)));

		std::vector<VkPresentModeKHR> presentModes(presentModeCount);
		vr = vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, presentModes.data());
		if (vr != VK_SUCCESS)
			return std::unexpected(std::format("Failed to vkGetPhysicalDeviceSurfacePresentModesKHR: {}", static_cast<int>(vr)));

		const VkSurfaceFormat2KHR surfaceFormat2 = SelectSwapSurfaceFormat(formats);
		const VkPresentModeKHR presentMode = SelectSwapPresentMode(gapi->IsCanDisableVSync(), presentModes);

		winSize = capabilities2.surfaceCapabilities.currentExtent;

		uint32_t minImageCount = capabilities2.surfaceCapabilities.minImageCount;
		uint32_t preferredImageCount = std::max(3u, minImageCount);

		uint32_t maxImageCount = (capabilities2.surfaceCapabilities.maxImageCount == 0) ?
			preferredImageCount :
			capabilities2.surfaceCapabilities.maxImageCount;

		m_MaxFramesInFlight = std::clamp(preferredImageCount, minImageCount, maxImageCount);
		m_ImageFormat = surfaceFormat2.surfaceFormat.format;

		const VkSwapchainCreateInfoKHR swapchainCreateInfo{
			.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
			.surface = surface,
			.minImageCount = m_MaxFramesInFlight,
			.imageFormat = surfaceFormat2.surfaceFormat.format,
			.imageColorSpace = surfaceFormat2.surfaceFormat.colorSpace,
			.imageExtent = capabilities2.surfaceCapabilities.currentExtent,
			.imageArrayLayers = 1,
			.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
			.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.preTransform = capabilities2.surfaceCapabilities.currentTransform,
			.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
			.presentMode = presentMode,
			.clipped = VK_TRUE,
		};
		vr = vkCreateSwapchainKHR(m_Device, &swapchainCreateInfo, nullptr, &m_SwapChain);
		if (vr != VK_SUCCESS)
			return std::unexpected(std::format("Failed to vkCreateSwapchainKHR: {}", static_cast<int>(vr)));

		{
			uint32_t imageCount = 0;
			vkGetSwapchainImagesKHR(m_Device, m_SwapChain, &imageCount, nullptr);
			ensure(m_MaxFramesInFlight <= imageCount, "Wrong swapchain setup");
			m_MaxFramesInFlight = imageCount;
		}
		std::vector<VkImage> swapImages(m_MaxFramesInFlight);
		vr = vkGetSwapchainImagesKHR(m_Device, m_SwapChain, &m_MaxFramesInFlight, swapImages.data());
		if (vr != VK_SUCCESS)
			return std::unexpected(std::format("Failed to vkGetSwapchainImagesKHR: {}", static_cast<int>(vr)));

		m_NextImages.resize(m_MaxFramesInFlight);
		VkImageViewCreateInfo imageViewCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = m_ImageFormat,
			.components = {.r = VK_COMPONENT_SWIZZLE_IDENTITY, .g = VK_COMPONENT_SWIZZLE_IDENTITY, .b = VK_COMPONENT_SWIZZLE_IDENTITY, .a = VK_COMPONENT_SWIZZLE_IDENTITY},
			.subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1},
		};
		for (uint32_t i = 0; i < m_MaxFramesInFlight; i++)
		{
			m_NextImages[i].image = swapImages[i];
			imageViewCreateInfo.image = m_NextImages[i].image;
			vr = vkCreateImageView(m_Device, &imageViewCreateInfo, nullptr, &m_NextImages[i].imageView);
			if (vr != VK_SUCCESS)
				return std::unexpected(std::format("Failed to vkCreateImageView: {}", static_cast<int>(vr)));
		}

		m_FrameResources.resize(m_MaxFramesInFlight);
		for (size_t i = 0; i < m_MaxFramesInFlight; ++i)
		{
			const VkSemaphoreCreateInfo semaphoreCreateInfo{ .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
			vr = vkCreateSemaphore(m_Device, &semaphoreCreateInfo, nullptr, &m_FrameResources[i].imageAvailableSemaphore);
			if (vr != VK_SUCCESS)
				return std::unexpected(std::format("Failed to vkCreateSemaphore(Available): {}", static_cast<int>(vr)));

			vr = vkCreateSemaphore(m_Device, &semaphoreCreateInfo, nullptr, &m_FrameResources[i].renderFinishedSemaphore);
			if (vr != VK_SUCCESS)
				return std::unexpected(std::format("Failed to vkCreateSemaphore(Finished): {}", static_cast<int>(vr)));
		}

		return winSize;
	}

	VkSurfaceFormat2KHR Swapchain_VK::SelectSwapSurfaceFormat(const std::vector<VkSurfaceFormat2KHR>& availableFormats) const
	{
		if (availableFormats.size() == 1 && availableFormats[0].surfaceFormat.format == VK_FORMAT_UNDEFINED)
		{
			VkSurfaceFormat2KHR result{
				.sType = VK_STRUCTURE_TYPE_SURFACE_FORMAT_2_KHR,
				.surfaceFormat = { VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR} };
			return result;
		}

		for (const auto& available : availableFormats)
		{
			if (available.surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				for (auto preferred : PREFERRED_FORMATS)
				{
					if (available.surfaceFormat.format == preferred)
						return available;
				}
			}
		}

		return availableFormats[0];
	}

	VkPresentModeKHR Swapchain_VK::SelectSwapPresentMode(bool vSync, const std::vector<VkPresentModeKHR>& availablePresentModes)
	{
		if (vSync)
			return VK_PRESENT_MODE_FIFO_KHR;

		bool mailboxSupported = false;
		bool immediateSupported = false;
		for (VkPresentModeKHR mode : availablePresentModes)
		{
			if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
				mailboxSupported = true;

			if (mode == VK_PRESENT_MODE_IMMEDIATE_KHR)
				immediateSupported = true;
		}

		if (mailboxSupported)
			return VK_PRESENT_MODE_MAILBOX_KHR;

		if (immediateSupported)
			return VK_PRESENT_MODE_IMMEDIATE_KHR;

		return VK_PRESENT_MODE_FIFO_KHR;
	}

	void Swapchain_VK::CmdTransitionSwapchainLayout(VkCommandBuffer cmd, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout)
	{
		VkPipelineStageFlags2 srcStage{ 0 };
		VkPipelineStageFlags2 dstStage{ 0 };
		VkAccessFlags2 srcAccess{ 0 };
		VkAccessFlags2 dstAccess{ 0 };

		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
		{
			srcStage = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
			srcAccess = VK_ACCESS_2_NONE;
			dstStage = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
			dstAccess = VK_ACCESS_2_NONE;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR && newLayout == VK_IMAGE_LAYOUT_GENERAL)
		{
			srcStage = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
			srcAccess = VK_ACCESS_2_NONE;
			dstStage = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_2_TRANSFER_BIT;
			dstAccess = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_GENERAL && newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
		{
			srcStage = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_2_TRANSFER_BIT;
			srcAccess = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT;
			dstStage = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
			dstAccess = VK_ACCESS_2_NONE;
		}
		else
			ensure(false, "Unsupported swapchain layout transition!");

		if (srcStage == 0 || dstStage == 0)
			ensure(false, "Invalid pipeline stage mask for swapchain layout transition!");

		const VkImageMemoryBarrier2 barrier
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
			.srcStageMask = srcStage,
			.srcAccessMask = srcAccess,
			.dstStageMask = dstStage,
			.dstAccessMask = dstAccess,
			.oldLayout = oldLayout,
			.newLayout = newLayout,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = image,
			.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}
		};

		const VkDependencyInfo depInfo{ .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO, .imageMemoryBarrierCount = 1, .pImageMemoryBarriers = &barrier };

		vkCmdPipelineBarrier2(cmd, &depInfo);
	}
}

#endif // Z_VULKAN
