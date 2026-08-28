#include "engine/gapi/vulkan/Swapchain_VK.h"
#include "engine/utils/EngineLogFlags.h"
#include "core/utils/VKToStringHelpers.h"

#if defined(Z_VULKAN)

namespace zzz::engine
{
	Swapchain_VK::Swapchain_VK(std::shared_ptr<VulkanAPI> gapi, std::shared_ptr<NativeWindow> window)
		: m_GAPI(std::move(gapi))
		, m_Window(std::move(window))
	{
		ensure(m_GAPI != nullptr, "VulkanAPI не должен быть null.");
		ensure(m_Window != nullptr, "NativeWindow не должен быть null.");
	}

	Swapchain_VK::~Swapchain_VK()
	{
		Release();
	}

	void Swapchain_VK::Initialize(VkSurfaceKHR surface)
	{
		m_Surface = surface;
		CreateSwapchain(m_Window->GetClientRect().GetSize());
	}

	void Swapchain_VK::Release()
	{
		CleanupSwapchain();
		m_Surface = VK_NULL_HANDLE;
	}

	void Swapchain_VK::CreateSwapchain(const Size2D<>& size)
	{
		VkDevice device = m_GAPI->GetDevice();
		VkPhysicalDevice physDevice = m_GAPI->GetPhysicalDevice();

		VkSurfaceCapabilitiesKHR capabilities;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physDevice, m_Surface, &capabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(physDevice, m_Surface, &formatCount, nullptr);
		std::vector<VkSurfaceFormatKHR> formats(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(physDevice, m_Surface, &formatCount, formats.data());

		VkSurfaceFormatKHR surfaceFormat = formats[0];
		for (const auto& availableFormat : formats)
		{
			if (availableFormat.format == VK_FORMAT_R8G8B8A8_UNORM || availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM)
			{
				surfaceFormat = availableFormat;
				break;
			}
		}
		m_Format = surfaceFormat.format;

		VkExtent2D extent = { static_cast<uint32_t>(size.GetWidth()), static_cast<uint32_t>(size.GetHeight()) };
		extent.width = std::clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		extent.height = std::clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		// Драйвер (особенно на Win32, где currentExtent обычно жёстко привязан к текущему размеру окна)
		// может вернуть capabilities, где min/maxImageExtent уже клампят extent к другому размеру, чем
		// запрошенный `size`. m_Size обязан отражать РЕАЛЬНЫЙ размер созданных VkImage/VkImageView, а не
		// запрошенный — иначе SurfView_VK будет строить renderArea по неверному размеру и Vulkan Validation
		// упадёт на несоответствии imageView/renderArea сразу после ресайза.
		m_Size = Size2D<>(extent.width, extent.height);

		uint32_t imageCount = capabilities.minImageCount + 1;
		if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount)
			imageCount = capabilities.maxImageCount;

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = m_Surface;
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

		uint32_t queueFamilyIndices[] = { m_GAPI->GetGraphicsQueueFamilyIndex(), m_GAPI->GetPresentQueueFamilyIndex() };
		if (queueFamilyIndices[0] != queueFamilyIndices[1])
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		}

		createInfo.preTransform = capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
		createInfo.clipped = VK_TRUE;

		VkResult vr = vkCreateSwapchainKHR(device, &createInfo, nullptr, &m_Swapchain);
		if (vr != VK_SUCCESS)
			THROW_RUNTIME("[Swapchain_VK::CreateSwapchain] Failed to create VkSwapchainKHR: 0x{:08X}", static_cast<uint32_t>(vr));

		m_GAPI->SetDebugName(m_Swapchain, "MainSwapchain");

		vkGetSwapchainImagesKHR(device, m_Swapchain, &imageCount, nullptr);
		m_Images.resize(imageCount);
		vkGetSwapchainImagesKHR(device, m_Swapchain, &imageCount, m_Images.data());

		m_ImageViews.resize(imageCount);
		for (size_t i = 0; i < imageCount; ++i)
		{
			VkImageViewCreateInfo viewInfo{};
			viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			viewInfo.image = m_Images[i];
			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			viewInfo.format = m_Format;
			viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			viewInfo.subresourceRange.baseMipLevel = 0;
			viewInfo.subresourceRange.levelCount = 1;
			viewInfo.subresourceRange.baseArrayLayer = 0;
			viewInfo.subresourceRange.layerCount = 1;

			vr = vkCreateImageView(device, &viewInfo, nullptr, &m_ImageViews[i]);
			if (vr != VK_SUCCESS)
				THROW_RUNTIME("[Swapchain_VK::CreateSwapchain] Failed to create VkImageView [{}]: 0x{:08X}", i, static_cast<uint32_t>(vr));

			m_GAPI->SetDebugName(m_ImageViews[i], std::format("SwapchainImageView[{}]", i).c_str());
		}

		DOut(!Z_LOG_GET(g_IsResizing), "[Swapchain_VK::CreateSwapchain] Created Swapchain {}x{} (Format: {}).", m_Size.GetWidth(), m_Size.GetHeight(), m_Format);
	}

	void Swapchain_VK::CleanupSwapchain()
	{
		VkDevice device = m_GAPI->GetDevice();
		if (!device)
			return;

		for (auto imageView : m_ImageViews)
		{
			if (imageView)
				vkDestroyImageView(device, imageView, nullptr);
		}
		m_ImageViews.clear();
		m_Images.clear();

		if (m_Swapchain)
		{
			vkDestroySwapchainKHR(device, m_Swapchain, nullptr);
			m_Swapchain = VK_NULL_HANDLE;
		}
	}

	VkImage Swapchain_VK::GetBackBuffer(uint32_t imageIndex) const noexcept
	{
		if (imageIndex < m_Images.size())
			return m_Images[imageIndex];
		return VK_NULL_HANDLE;
	}

	VkImageView Swapchain_VK::GetImageView(uint32_t imageIndex) const noexcept
	{
		if (imageIndex < m_ImageViews.size())
			return m_ImageViews[imageIndex];
		return VK_NULL_HANDLE;
	}

	void Swapchain_VK::Present(bool vSync, uint32_t imageIndex, VkSemaphore waitSemaphore)
	{
		(void)vSync;
		if (!m_Swapchain)
			return;

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		if (waitSemaphore != VK_NULL_HANDLE)
		{
			presentInfo.waitSemaphoreCount = 1;
			presentInfo.pWaitSemaphores = &waitSemaphore;
		}
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &m_Swapchain;
		presentInfo.pImageIndices = &imageIndex;

		vkQueuePresentKHR(m_GAPI->GetPresentQueue(), &presentInfo);
	}

	void Swapchain_VK::OnResize(const Size2D<>& size)
	{
		if (!m_Swapchain)
			return;

		m_GAPI->WaitForGpu();
		CleanupSwapchain();
		CreateSwapchain(size);

		DOut(!Z_LOG_GET(g_IsResizing), "[Swapchain_VK::OnResize] Successfully resized to {}x{} (BackBufferFormat: {}).", m_Size.GetWidth(), m_Size.GetHeight(), m_Format);
	}
}

#endif // Z_VULKAN
