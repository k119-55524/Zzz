#include "engine/gapi/vulkan/DepthBuffer_VK.h"
#include "engine/utils/EngineLogFlags.h"
#include "core/utils/platforms/VKToStringHelpers.h"

Z_SET_LOG_CATEGORY(::zzz::core::GAPI);

#if defined(Z_VULKAN)

namespace zzz::engine
{
	DepthBuffer_VK::DepthBuffer_VK(std::shared_ptr<VulkanAPI> gapi, const Size2D<>& size)
		: m_GAPI(std::move(gapi))
	{
		ensure(m_GAPI != nullptr, "VulkanAPI не должен быть null.");
		CreateDepthResources(size);
	}

	DepthBuffer_VK::~DepthBuffer_VK()
	{
		Release();
	}

	void DepthBuffer_VK::Release()
	{
		VkDevice device = m_GAPI->GetDevice();
		if (!device)
			return;

		if (m_ImageView)
		{
			vkDestroyImageView(device, m_ImageView, nullptr);
			m_ImageView = VK_NULL_HANDLE;
		}

		if (m_Image)
		{
			vkDestroyImage(device, m_Image, nullptr);
			m_Image = VK_NULL_HANDLE;
		}

		if (m_Memory)
		{
			vkFreeMemory(device, m_Memory, nullptr);
			m_Memory = VK_NULL_HANDLE;
		}
	}

	void DepthBuffer_VK::CreateDepthResources(const Size2D<>& size)
	{
		m_Size = size;
		VkDevice device = m_GAPI->GetDevice();
		VkPhysicalDevice physDevice = m_GAPI->GetPhysicalDevice();

		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = static_cast<uint32_t>(m_Size.width);
		imageInfo.extent.height = static_cast<uint32_t>(m_Size.height);
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = m_Format;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VkResult vr = vkCreateImage(device, &imageInfo, nullptr, &m_Image);
		if (vr != VK_SUCCESS)
			THROW_RUNTIME("[DepthBuffer_VK::CreateDepthResources] Failed to create depth VkImage: 0x{:08X}", static_cast<uint32_t>(vr));

		m_GAPI->SetDebugName(m_Image, "DepthImage");

		VkMemoryRequirements memReqs;
		vkGetImageMemoryRequirements(device, m_Image, &memReqs);

		VkPhysicalDeviceMemoryProperties memProps;
		vkGetPhysicalDeviceMemoryProperties(physDevice, &memProps);

		uint32_t memoryTypeIndex = UINT32_MAX;
		for (uint32_t i = 0; i < memProps.memoryTypeCount; ++i)
		{
			if ((memReqs.memoryTypeBits & (1 << i)) && (memProps.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT))
			{
				memoryTypeIndex = i;
				break;
			}
		}

		if (memoryTypeIndex == UINT32_MAX)
			THROW_RUNTIME("[DepthBuffer_VK::CreateDepthResources] Failed to find suitable memory type for depth image.");

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memReqs.size;
		allocInfo.memoryTypeIndex = memoryTypeIndex;

		vr = vkAllocateMemory(device, &allocInfo, nullptr, &m_Memory);
		if (vr != VK_SUCCESS)
			THROW_RUNTIME("[DepthBuffer_VK::CreateDepthResources] Failed to allocate memory for depth image: 0x{:08X}", static_cast<uint32_t>(vr));

		m_GAPI->SetDebugName(m_Memory, "DepthImageMemory");

		vkBindImageMemory(device, m_Image, m_Memory, 0);

		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = m_Image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = m_Format;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		if (m_Format == VK_FORMAT_D24_UNORM_S8_UINT || m_Format == VK_FORMAT_D32_SFLOAT_S8_UINT)
			viewInfo.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		vr = vkCreateImageView(device, &viewInfo, nullptr, &m_ImageView);
		if (vr != VK_SUCCESS)
			THROW_RUNTIME("[DepthBuffer_VK::CreateDepthResources] Failed to create VkImageView for depth image: 0x{:08X}", static_cast<uint32_t>(vr));

		m_GAPI->SetDebugName(m_ImageView, "DepthImageView");

		DOut(!Z_LOG_GET(g_IsResizing), "[DepthBuffer_VK::CreateDepthResources] Created DepthBuffer {}x{} (DepthFormat: {}).", m_Size.width, m_Size.height, m_Format);
	}

	void DepthBuffer_VK::OnResize(const Size2D<>& size)
	{
		Release();
		CreateDepthResources(size);

		DOut(!Z_LOG_GET(g_IsResizing), "[DepthBuffer_VK::OnResize] Successfully resized DepthBuffer to {}x{} (DepthFormat: {}).", m_Size.width, m_Size.height, m_Format);
	}
}

#endif // Z_VULKAN
