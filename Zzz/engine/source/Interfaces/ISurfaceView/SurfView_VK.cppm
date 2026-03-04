export module SurfView_VK;

#if defined(ZRENDER_API_VULKAN)

import Math;
import IPSO;
import IGAPI;
import VKAPI;
import Scene;
import Ensure;
import Result;
import Size2D;
import Colors;
import Camera;
import IAppWin;
import Helpers;
import IMeshGPU;
import ISurfView;
import StrConvert;
import DebugOutput;
import RenderQueue;
import RenderVolume;
import ViewportDesc;
import AppWin_MSWin;
import EngineConstants;
import PrimitiveTopology;

using namespace zzz::math;
using namespace zzz::core;
using namespace zzz::logger;
using namespace zzz::colors;

namespace zzz::vk
{
	// Структуры для uniform buffers
	struct GPU_LayerConstants
	{
		Matrix4x4 WorldViewProj;
		Matrix4x4 View;
		Matrix4x4 Proj;
		Matrix4x4 ViewProj;
	};

	struct GPU_MaterialConstants
	{
		Vector4 BaseColor;
		float Roughness;
		float Metallic;
		float Padding[2];
	};

	struct GPU_ObjectConstants
	{
		Matrix4x4 World;
		Matrix4x4 WorldViewProj;
	};

	// Константы для preferred форматов
	constexpr std::array<VkFormat, 3> PREFERRED_SWAPCHAIN_FORMATS = {
		VK_FORMAT_B8G8R8A8_SRGB,
		VK_FORMAT_R8G8B8A8_SRGB,
		VK_FORMAT_A2B10G10R10_UNORM_PACK32
	};

	constexpr std::array<VkFormat, 3> PREFERRED_DEPTH_FORMATS = {
		VK_FORMAT_D32_SFLOAT,
		VK_FORMAT_D24_UNORM_S8_UINT,
		VK_FORMAT_D16_UNORM
	};

	export class SurfView_VK final : public ISurfView
	{
		Z_NO_COPY_MOVE(SurfView_VK);

	public:
		explicit SurfView_VK(std::shared_ptr<IAppWin> _iAppWin, std::shared_ptr<IGAPI> _iGAPI);
		~SurfView_VK() override;

		[[nodiscard]] Result<> Initialize() override;
		void PrepareFrame(const std::shared_ptr<RenderQueue> renderQueue) override;
		void RenderFrame() override;
		void OnResize(const Size2D<>& size) override;
		[[nodiscard]] Result<> OnUpdateVSyncState() override;

		void PreRender() override;
		void PostRender() override;

	private:
		// Core members
		std::shared_ptr<AppWin_MSWin> m_iAppWin;
		std::shared_ptr<VKAPI> m_VulkanAPI;

		// Surface and swapchain
		VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
		VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;
		VkExtent2D m_SwapchainExtent{};
		std::vector<VkImage> m_SwapchainImages;
		std::vector<VkImageView> m_SwapchainImageViews;
		std::vector<VkFramebuffer> m_Framebuffers;
		VkFormat m_ChosenSwapchainFormat = VK_FORMAT_UNDEFINED;

		// Depth resources
		VkFormat m_ChosenDepthFormat = VK_FORMAT_UNDEFINED;
		VkImage m_DepthImage = VK_NULL_HANDLE;
		VkDeviceMemory m_DepthImageMemory = VK_NULL_HANDLE;
		VkImageView m_DepthImageView = VK_NULL_HANDLE;

		// Render pass
		VkRenderPass m_RenderPass = VK_NULL_HANDLE;

		// Queue families
		uint32_t m_GraphicsQueueFamily = UINT32_MAX;
		uint32_t m_PresentQueueFamily = UINT32_MAX;

		// Command pool and per-frame data
		VkCommandPool m_CommandPool = VK_NULL_HANDLE;

		struct FrameData
		{
			VkFence fence = VK_NULL_HANDLE;					// Сигналит когда ГПУ закончил рендер фрейма
			VkSemaphore acquireSemaphore = VK_NULL_HANDLE;	// для vkAcquireNextImageKHR
			VkSemaphore renderSemaphore = VK_NULL_HANDLE;	// для vkQueuePresentKHR
			VkCommandBuffer cmdBuffer = VK_NULL_HANDLE;		// Буфер команд для этого фрейма
			uint32_t imageIndex = 0;						// Индекс изображения в свапчейне
			bool inFlight = false;							// Флаг что фрейм в полете
		};

		std::array<FrameData, FRAMES_IN_FLIGHT> m_Frames;
		std::mutex m_SwapchainMutex;

		VkSemaphore m_ImageAvailableSemaphore = VK_NULL_HANDLE;  // Один на всех
		VkSemaphore m_RenderFinishedSemaphore = VK_NULL_HANDLE;  // Один на всех

		// Private methods
		[[nodiscard]] Result<> FindQueueFamilies();
		[[nodiscard]] Result<> CreateSurface();
		[[nodiscard]] Result<> CreateSwapchain(const Size2D<>& size);
		[[nodiscard]] Result<> CreateImageViews();
		[[nodiscard]] Result<> CreateRenderPass();
		[[nodiscard]] Result<> CreateDepthResources(const Size2D<>& size);
		[[nodiscard]] Result<> CreateFramebuffers();
		[[nodiscard]] Result<> CreateCommandPool();
		[[nodiscard]] Result<> CreateSyncObjects();
		[[nodiscard]] Result<> RecreateSwapchain();
		void CleanupSwapchain();
		[[nodiscard]] uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	};

	SurfView_VK::SurfView_VK(
		std::shared_ptr<IAppWin> _iAppWin,
		std::shared_ptr<IGAPI> _iGAPI)
		: ISurfView(_iGAPI)
		, m_iAppWin{ std::dynamic_pointer_cast<AppWin_MSWin>(_iAppWin) }
		, m_VulkanAPI{ std::dynamic_pointer_cast<VKAPI>(_iGAPI) }
	{
		ensure(m_iAppWin, "App window must be of type AppWin_MSWin.");
		ensure(m_VulkanAPI, "Failed to cast IGAPI to VKAPI.");
	}

	SurfView_VK::~SurfView_VK()
	{
		VkDevice device = m_VulkanAPI->GetDevice();
		if (!device)
			return;

		vkDeviceWaitIdle(device);

		if (m_ImageAvailableSemaphore)
			vkDestroySemaphore(device, m_ImageAvailableSemaphore, nullptr);
		if (m_RenderFinishedSemaphore)
			vkDestroySemaphore(device, m_RenderFinishedSemaphore, nullptr);

		// Cleanup per-frame data
		for (auto& frame : m_Frames)
		{
			if (frame.fence)
				vkDestroyFence(device, frame.fence, nullptr);
		}

		CleanupSwapchain();

		if (m_RenderPass)
			vkDestroyRenderPass(device, m_RenderPass, nullptr);

		if (m_CommandPool)
			vkDestroyCommandPool(device, m_CommandPool, nullptr);

		if (m_Surface)
			vkDestroySurfaceKHR(m_VulkanAPI->GetInstance(), m_Surface, nullptr);
	}

#pragma region Initialize
	Result<> SurfView_VK::Initialize()
	{
		auto winSize = m_iAppWin->GetWinSize();
		m_SurfSize.SetFrom(winSize);

		auto res = CreateSurface()
			.and_then([&]() { return FindQueueFamilies(); })
			.and_then([&]() { return CreateSwapchain(winSize); })
			.and_then([&]() { return CreateCommandPool(); })
			.and_then([&]() { return CreateImageViews(); })
			.and_then([&]() { return CreateRenderPass(); })
			.and_then([&]() { return CreateDepthResources(winSize); })
			.and_then([&]() { return CreateFramebuffers(); })
			.and_then([&]() { return CreateSyncObjects(); });

		if (!res)
			return UNEXPECTED(eResult::failure, L"Failed to initialize SurfView: {}", res.error().getMessage());

		return {};
	}

	[[nodiscard]] Result<> SurfView_VK::FindQueueFamilies()
	{
		VkPhysicalDevice physicalDevice = m_VulkanAPI->GetPhysicalDevice();

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

		// Find graphics and present queue families
		for (uint32_t i = 0; i < queueFamilyCount; i++)
		{
			if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				m_GraphicsQueueFamily = i;

				// Check if this queue supports presentation
				VkBool32 presentSupport = VK_FALSE;
				vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, m_Surface, &presentSupport);
				if (presentSupport)
				{
					m_PresentQueueFamily = i;
					break;
				}
			}
		}

		// If we didn't find a combined queue, look for a separate present queue
		if (m_PresentQueueFamily == UINT32_MAX)
		{
			for (uint32_t i = 0; i < queueFamilyCount; i++)
			{
				VkBool32 presentSupport = VK_FALSE;
				vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, m_Surface, &presentSupport);
				if (presentSupport)
				{
					m_PresentQueueFamily = i;
					break;
				}
			}
		}

		if (m_GraphicsQueueFamily == UINT32_MAX || m_PresentQueueFamily == UINT32_MAX)
			return UNEXPECTED(eResult::failure, L"Failed to find required queue families");

		return {};
	}

	[[nodiscard]] Result<> SurfView_VK::CreateSurface()
	{
		VkResult vr = VK_SUCCESS;

#if defined(ZPLATFORM_MSWINDOWS)
		VkWin32SurfaceCreateInfoKHR createInfo{
			.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
			.hinstance = GetModuleHandle(nullptr),
			.hwnd = m_iAppWin->GetHWND()
		};
		vr = vkCreateWin32SurfaceKHR(m_VulkanAPI->GetInstance(), &createInfo, nullptr, &m_Surface);
#elif defined(ZPLATFORM_ANDROID)
		// Android implementation
		VkAndroidSurfaceCreateInfoKHR createInfo{
			.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR,
			.window = GetNativeWindow() // You need to implement this
		};
		vr = vkCreateAndroidSurfaceKHR(m_VulkanAPI->GetInstance(), &createInfo, nullptr, &m_Surface);
#elif defined(ZPLATFORM_LINUX)
#if defined(USE_WAYLAND)
		VkWaylandSurfaceCreateInfoKHR createInfo{
			.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR,
			.display = GetWaylandDisplay(),
			.surface = GetWaylandSurface()
		};
		vr = vkCreateWaylandSurfaceKHR(m_VulkanAPI->GetInstance(), &createInfo, nullptr, &m_Surface);
#else
		VkXcbSurfaceCreateInfoKHR createInfo{
			.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR,
			.connection = GetXcbConnection(),
			.window = GetXcbWindow()
		};
		vr = vkCreateXcbSurfaceKHR(m_VulkanAPI->GetInstance(), &createInfo, nullptr, &m_Surface);
#endif
#else
#error "Platform not supported"
#endif

		if (vr != VK_SUCCESS)
			return UNEXPECTED(eResult::failure, L"Failed to create surface ({})", static_cast<int>(vr));

		return {};
	}

	[[nodiscard]] Result<> SurfView_VK::CreateSwapchain(const Size2D<>& size)
	{
		VkPhysicalDevice physicalDevice = m_VulkanAPI->GetPhysicalDevice();
		VkDevice device = m_VulkanAPI->GetDevice();

		// Get surface capabilities
		VkSurfaceCapabilitiesKHR capabilities;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, m_Surface, &capabilities);

		// Calculate extent
		if (capabilities.currentExtent.width != UINT32_MAX)
		{
			m_SwapchainExtent = capabilities.currentExtent;
		}
		else
		{
			m_SwapchainExtent = {
				std::clamp(static_cast<uint32_t>(size.width),
						  capabilities.minImageExtent.width,
						  capabilities.maxImageExtent.width),
				std::clamp(static_cast<uint32_t>(size.height),
						  capabilities.minImageExtent.height,
						  capabilities.maxImageExtent.height)
			};
		}

		// Get surface formats
		uint32_t formatCount = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, m_Surface, &formatCount, nullptr);
		std::vector<VkSurfaceFormatKHR> formats(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, m_Surface, &formatCount, formats.data());

		// Choose format
		m_ChosenSwapchainFormat = formats[0].format; // fallback
		for (auto& preferredFormat : PREFERRED_SWAPCHAIN_FORMATS)
		{
			for (auto& format : formats)
			{
				if (format.format == preferredFormat &&
					format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
				{
					m_ChosenSwapchainFormat = format.format;
					break;
				}
			}
		}

		// Determine image count
		uint32_t imageCount = BACK_BUFFER_COUNT;
		if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount)
			imageCount = capabilities.maxImageCount;

		// Choose present mode based on vsync
		VkPresentModeKHR presentMode = m_GAPI->IsVSyncEnabled()
			? VK_PRESENT_MODE_FIFO_KHR
			: VK_PRESENT_MODE_MAILBOX_KHR;

		// Create swapchain
		VkSwapchainCreateInfoKHR createInfo{
			.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
			.surface = m_Surface,
			.minImageCount = imageCount,
			.imageFormat = m_ChosenSwapchainFormat,
			.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
			.imageExtent = m_SwapchainExtent,
			.imageArrayLayers = 1,
			.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.preTransform = capabilities.currentTransform,
			.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
			.presentMode = presentMode,
			.clipped = VK_TRUE,
			.oldSwapchain = VK_NULL_HANDLE
		};

		VkResult vr = vkCreateSwapchainKHR(device, &createInfo, nullptr, &m_Swapchain);
		if (vr != VK_SUCCESS)
			return UNEXPECTED(eResult::failure, L"Failed to create swapchain ({})", static_cast<int>(vr));

		// Get swapchain images
		vkGetSwapchainImagesKHR(device, m_Swapchain, &imageCount, nullptr);
		m_SwapchainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(device, m_Swapchain, &imageCount, m_SwapchainImages.data());

		return {};
	}

	[[nodiscard]] Result<> SurfView_VK::CreateCommandPool()
	{
		ensure(m_GraphicsQueueFamily != UINT32_MAX, "Graphics queue family not found");

		VkDevice device = m_VulkanAPI->GetDevice();
		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = m_GraphicsQueueFamily;

		VkResult vr = vkCreateCommandPool(device, &poolInfo, nullptr, &m_CommandPool);
		if (vr != VK_SUCCESS)
			return UNEXPECTED(eResult::failure, L"Failed to create command pool ({})", static_cast<int>(vr));

		return {};
	}

	[[nodiscard]] Result<> SurfView_VK::CreateImageViews()
	{
		VkDevice device = m_VulkanAPI->GetDevice();
		m_SwapchainImageViews.resize(m_SwapchainImages.size());

		for (size_t i = 0; i < m_SwapchainImages.size(); i++)
		{
			VkImageViewCreateInfo createInfo{
				.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
				.image = m_SwapchainImages[i],
				.viewType = VK_IMAGE_VIEW_TYPE_2D,
				.format = m_ChosenSwapchainFormat,
				.components = {
					.r = VK_COMPONENT_SWIZZLE_IDENTITY,
					.g = VK_COMPONENT_SWIZZLE_IDENTITY,
					.b = VK_COMPONENT_SWIZZLE_IDENTITY,
					.a = VK_COMPONENT_SWIZZLE_IDENTITY
				},
				.subresourceRange = {
					.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
					.baseMipLevel = 0,
					.levelCount = 1,
					.baseArrayLayer = 0,
					.layerCount = 1
				}
			};

			VkResult vr = vkCreateImageView(device, &createInfo, nullptr, &m_SwapchainImageViews[i]);
			if (vr != VK_SUCCESS)
				return UNEXPECTED(eResult::failure, L"Failed to create image view {} ({})", i, static_cast<int>(vr));
		}

		return {};
	}

	[[nodiscard]] Result<> SurfView_VK::CreateRenderPass()
	{
		VkPhysicalDevice physicalDevice = m_VulkanAPI->GetPhysicalDevice();
		VkDevice device = m_VulkanAPI->GetDevice();

		// Color attachment
		VkAttachmentDescription colorAttachment{
			.format = m_ChosenSwapchainFormat,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
		};

		// Choose depth format
		for (auto fmt : PREFERRED_DEPTH_FORMATS)
		{
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(physicalDevice, fmt, &props);
			if (props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
			{
				m_ChosenDepthFormat = fmt;
				break;
			}
		}

		if (m_ChosenDepthFormat == VK_FORMAT_UNDEFINED)
			return UNEXPECTED(eResult::failure, L"No suitable depth format found");

		// Depth attachment
		VkAttachmentDescription depthAttachment{
			.format = m_ChosenDepthFormat,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
		};

		VkAttachmentReference colorAttachmentRef{
			.attachment = 0,
			.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		};

		VkAttachmentReference depthAttachmentRef{
			.attachment = 1,
			.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
		};

		VkSubpassDescription subpass{
			.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
			.colorAttachmentCount = 1,
			.pColorAttachments = &colorAttachmentRef,
			.pDepthStencilAttachment = &depthAttachmentRef
		};

		VkSubpassDependency dependency{
			.srcSubpass = VK_SUBPASS_EXTERNAL,
			.dstSubpass = 0,
			.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
						   VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
			.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
						   VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
			.srcAccessMask = 0,
			.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
							VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT
		};

		std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
		VkRenderPassCreateInfo renderPassInfo{
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
			.attachmentCount = static_cast<uint32_t>(attachments.size()),
			.pAttachments = attachments.data(),
			.subpassCount = 1,
			.pSubpasses = &subpass,
			.dependencyCount = 1,
			.pDependencies = &dependency
		};

		VkResult vr = vkCreateRenderPass(device, &renderPassInfo, nullptr, &m_RenderPass);
		if (vr != VK_SUCCESS)
			return UNEXPECTED(eResult::failure, L"Failed to create render pass ({})", static_cast<int>(vr));

		return {};
	}

	[[nodiscard]] Result<> SurfView_VK::CreateDepthResources(const Size2D<>& size)
	{
		VkDevice device = m_VulkanAPI->GetDevice();

		VkImageCreateInfo imageInfo{
			.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			.imageType = VK_IMAGE_TYPE_2D,
			.format = m_ChosenDepthFormat,
			.extent = {
				.width = m_SwapchainExtent.width,
				.height = m_SwapchainExtent.height,
				.depth = 1
			},
			.mipLevels = 1,
			.arrayLayers = 1,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.tiling = VK_IMAGE_TILING_OPTIMAL,
			.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
		};

		VkResult vr = vkCreateImage(device, &imageInfo, nullptr, &m_DepthImage);
		if (vr != VK_SUCCESS)
			return UNEXPECTED(eResult::failure, L"Failed to create depth image ({})", static_cast<int>(vr));

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(device, m_DepthImage, &memRequirements);

		VkMemoryAllocateInfo allocInfo{
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.allocationSize = memRequirements.size,
			.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits,
											  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
		};

		vr = vkAllocateMemory(device, &allocInfo, nullptr, &m_DepthImageMemory);
		if (vr != VK_SUCCESS)
			return UNEXPECTED(eResult::failure, L"Failed to allocate depth image memory ({})", static_cast<int>(vr));

		vkBindImageMemory(device, m_DepthImage, m_DepthImageMemory, 0);

		VkImageViewCreateInfo viewInfo{
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = m_DepthImage,
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = m_ChosenDepthFormat,
			.subresourceRange = {
				.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1
			}
		};

		vr = vkCreateImageView(device, &viewInfo, nullptr, &m_DepthImageView);
		if (vr != VK_SUCCESS)
			return UNEXPECTED(eResult::failure, L"Failed to create depth image view ({})", static_cast<int>(vr));

		return {};
	}

	[[nodiscard]] Result<> SurfView_VK::CreateFramebuffers()
	{
		VkDevice device = m_VulkanAPI->GetDevice();
		m_Framebuffers.resize(m_SwapchainImageViews.size());

		for (size_t i = 0; i < m_SwapchainImageViews.size(); i++)
		{
			std::array<VkImageView, 2> attachments = {
				m_SwapchainImageViews[i],
				m_DepthImageView
			};

			VkFramebufferCreateInfo framebufferInfo{
				.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
				.renderPass = m_RenderPass,
				.attachmentCount = static_cast<uint32_t>(attachments.size()),
				.pAttachments = attachments.data(),
				.width = m_SwapchainExtent.width,
				.height = m_SwapchainExtent.height,
				.layers = 1
			};

			VkResult vr = vkCreateFramebuffer(device, &framebufferInfo, nullptr, &m_Framebuffers[i]);
			if (vr != VK_SUCCESS)
				return UNEXPECTED(eResult::failure, L"Failed to create framebuffer {} ({})", i, static_cast<int>(vr));
		}

		return {};
	}

	Result<> SurfView_VK::CreateSyncObjects()
	{
		VkDevice device = m_VulkanAPI->GetDevice();

		// Create semaphores
		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		VkResult vr = vkCreateSemaphore(device, &semaphoreInfo, nullptr, &m_ImageAvailableSemaphore);
		if (vr != VK_SUCCESS)
			return UNEXPECTED(eResult::failure, L"Failed to create m_ImageAvailableSemaphore semaphore.");

		vr = vkCreateSemaphore(device, &semaphoreInfo, nullptr, &m_RenderFinishedSemaphore);
		if (vr != VK_SUCCESS)
			return UNEXPECTED(eResult::failure, L"Failed to create m_RenderFinishedSemaphore semaphore.");

		for (uint32_t i = 0; i < FRAMES_IN_FLIGHT; i++)
		{
			// Allocate command buffer
			VkCommandBufferAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocInfo.commandPool = m_CommandPool;
			allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			allocInfo.commandBufferCount = 1;

			vr = vkAllocateCommandBuffers(device, &allocInfo, &m_Frames[i].cmdBuffer);
			if (vr != VK_SUCCESS)
				return UNEXPECTED(eResult::failure, L"Failed to allocate command buffer for frame {}", i);

			VkSemaphoreCreateInfo semInfo = {};
			semInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
			vkCreateSemaphore(device, &semInfo, nullptr, &m_Frames[i].acquireSemaphore);
			vkCreateSemaphore(device, &semInfo, nullptr, &m_Frames[i].renderSemaphore);

			VkFenceCreateInfo fenceInfo{};
			fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
			vr = vkCreateFence(device, &fenceInfo, nullptr, &m_Frames[i].fence);
			if (vr != VK_SUCCESS)
				return UNEXPECTED(eResult::failure, L"Failed to create fence for frame {}", i);

			m_Frames[i].inFlight = false;
		}

		return {};
	}

	[[nodiscard]] uint32_t SurfView_VK::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(m_VulkanAPI->GetPhysicalDevice(), &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
		{
			if ((typeFilter & (1 << i)) &&
				(memProperties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				return i;
			}
		}

		throw_runtime_error("Failed to find suitable memory type");
		return UINT32_MAX;
	}

	void SurfView_VK::CleanupSwapchain()
	{
		VkDevice device = m_VulkanAPI->GetDevice();

		for (auto framebuffer : m_Framebuffers)
			vkDestroyFramebuffer(device, framebuffer, nullptr);
		m_Framebuffers.clear();

		if (m_DepthImageView)
		{
			vkDestroyImageView(device, m_DepthImageView, nullptr);
			m_DepthImageView = VK_NULL_HANDLE;
		}

		if (m_DepthImage)
		{
			vkDestroyImage(device, m_DepthImage, nullptr);
			m_DepthImage = VK_NULL_HANDLE;
		}

		if (m_DepthImageMemory)
		{
			vkFreeMemory(device, m_DepthImageMemory, nullptr);
			m_DepthImageMemory = VK_NULL_HANDLE;
		}

		for (auto imageView : m_SwapchainImageViews)
			vkDestroyImageView(device, imageView, nullptr);
		m_SwapchainImageViews.clear();

		if (m_Swapchain)
		{
			vkDestroySwapchainKHR(device, m_Swapchain, nullptr);
			m_Swapchain = VK_NULL_HANDLE;
		}
	}

	[[nodiscard]] Result<> SurfView_VK::RecreateSwapchain()
	{
		VkDevice device = m_VulkanAPI->GetDevice();
		VkPhysicalDevice physicalDevice = m_VulkanAPI->GetPhysicalDevice();

		ensure(device, "Logical device is not initialized.");
		ensure(physicalDevice, "Physical device is not initialized.");

		// Wait for GPU to finish
		vkDeviceWaitIdle(device);

		// Get current surface capabilities
		VkSurfaceCapabilitiesKHR capabilities{};
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, m_Surface, &capabilities);

		// Calculate new extent
		VkExtent2D newExtent{};
		if (capabilities.currentExtent.width != UINT32_MAX)
		{
			newExtent = capabilities.currentExtent;
		}
		else
		{
			newExtent.width = std::clamp(
				static_cast<uint32_t>(m_SurfSize.width),
				capabilities.minImageExtent.width,
				capabilities.maxImageExtent.width);

			newExtent.height = std::clamp(
				static_cast<uint32_t>(m_SurfSize.height),
				capabilities.minImageExtent.height,
				capabilities.maxImageExtent.height);
		}

		if (newExtent.width == 0 || newExtent.height == 0)
			return {}; // Window is minimized

		m_SwapchainExtent = newExtent;

		// Save old swapchain
		VkSwapchainKHR oldSwapchain = m_Swapchain;

		// Clean up old resources
		CleanupSwapchain();

		// Get present modes
		uint32_t presentModeCount = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, m_Surface, &presentModeCount, nullptr);
		std::vector<VkPresentModeKHR> presentModes(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, m_Surface, &presentModeCount, presentModes.data());

		// Choose present mode based on vsync
		bool wantVSync = m_GAPI->IsVSyncEnabled();
		VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR; // always available
		if (!wantVSync)
		{
			for (auto mode : presentModes)
			{
				if (mode == VK_PRESENT_MODE_IMMEDIATE_KHR)
				{
					presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
					break;
				}
			}
		}

		// Get image count
		uint32_t imageCount = BACK_BUFFER_COUNT;
		if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount)
			imageCount = capabilities.maxImageCount;

		// Create new swapchain
		VkSwapchainCreateInfoKHR createInfo{
			.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
			.surface = m_Surface,
			.minImageCount = imageCount,
			.imageFormat = m_ChosenSwapchainFormat,
			.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
			.imageExtent = newExtent,
			.imageArrayLayers = 1,
			.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.preTransform = capabilities.currentTransform,
			.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
			.presentMode = presentMode,
			.clipped = VK_TRUE,
			.oldSwapchain = oldSwapchain
		};

		VkResult vr = vkCreateSwapchainKHR(device, &createInfo, nullptr, &m_Swapchain);
		if (vr != VK_SUCCESS)
			return UNEXPECTED(eResult::failure, L"Failed to recreate swapchain ({})", static_cast<int>(vr));

		// Destroy old swapchain
		if (oldSwapchain)
			vkDestroySwapchainKHR(device, oldSwapchain, nullptr);

		// Get new swapchain images
		vkGetSwapchainImagesKHR(device, m_Swapchain, &imageCount, nullptr);
		m_SwapchainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(device, m_Swapchain, &imageCount, m_SwapchainImages.data());

		// Recreate dependent resources
		return CreateImageViews()
			.and_then([&] { return CreateDepthResources(Size2D<>(newExtent.width, newExtent.height)); })
			.and_then([&] { return CreateFramebuffers(); });
	}
#pragma endregion

	void Fill()
	{
		//if (stepCounter == 1)
		//{
		//	//zU32 indexRender = m_VulkanAPI->GetIndexFrameRender();
		//	zU32 indexRender = m_VulkanAPI->GetIndexFramePrepare();

		//	DebugOutputLite(L">>>>> #1 PreRender(). stepCounter: {}, indexRender: {}, imageIndex: {}.", stepCounter, indexRender, imageIndex);

		//	auto& frameRender = m_Frames[indexRender];
		//	vr = vkResetCommandBuffer(frameRender.cmdBuffer, 0);
		//	if (vr != VK_SUCCESS)
		//		throw_runtime_error("Failed to vkResetCommandBuffer().");

		//	VkCommandBufferBeginInfo bi{};
		//	bi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		//	bi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		//	vr = vkBeginCommandBuffer(frameRender.cmdBuffer, &bi);
		//	if (vr != VK_SUCCESS)
		//		throw_runtime_error("Failed to vkBeginCommandBuffer().");

		//	// ничего не записываем — пустой буфер
		//	vr = vkEndCommandBuffer(frameRender.cmdBuffer);
		//	if (vr != VK_SUCCESS)
		//		throw_runtime_error("Failed to vkEndCommandBuffer().");

		//	frameRender.inFlight = true;
		//}
	}

#pragma region Rendering
	void SurfView_VK::PreRender()
	{
		static zU32 stepCounter = 0;
		stepCounter++;

		VkDevice device = m_VulkanAPI->GetDevice();
		zU32 indexPrepare = m_VulkanAPI->GetIndexFramePrepare();
		auto& frame = m_Frames[indexPrepare];

		VkResult vr = vkWaitForFences(device, 1, &frame.fence, VK_TRUE, UINT64_MAX);
		if (vr != VK_SUCCESS)
			throw_runtime_error("Failed to vkWaitForFences().");

		uint32_t imageIndex;
		vr = vkAcquireNextImageKHR(device, m_Swapchain, UINT64_MAX, frame.acquireSemaphore, VK_NULL_HANDLE, &imageIndex);
		if (vr == VK_ERROR_OUT_OF_DATE_KHR) {
			if (!RecreateSwapchain())
				throw_runtime_error("Failed to RecreateSwapchain().");

			return;
		}
		else if (vr != VK_SUCCESS && vr != VK_SUBOPTIMAL_KHR)
		{
			throw_runtime_error("Failed to vkAcquireNextImageKHR().");
		}

		DebugOutputLite(L">>>>> #0 PreRender(). stepCounter: {}, indexPrepare: {}, imageIndex: {}.", stepCounter, indexPrepare, imageIndex);

		frame.imageIndex = imageIndex;
	}

	void SurfView_VK::PrepareFrame(const std::shared_ptr<RenderQueue> renderQueue)
	{
		static zU32 stepCounter = 0;
		stepCounter++;

		VkDevice device = m_VulkanAPI->GetDevice();
		zU32 indexPrepare = m_VulkanAPI->GetIndexFramePrepare();
		auto& frame = m_Frames[indexPrepare];

		DebugOutputLite(L">>>>> PrepareFrame(). stepCounter: {}, indexPrepare: {}, frame.imageIndex: {}.", stepCounter, indexPrepare, frame.imageIndex);

		VkResult vr = vkResetCommandBuffer(frame.cmdBuffer, 0);
		if (vr != VK_SUCCESS)
			throw_runtime_error("Failed to vkResetCommandBuffer().");

		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		vr = vkBeginCommandBuffer(frame.cmdBuffer, &beginInfo);
		if (vr != VK_SUCCESS)
			throw_runtime_error("Failed to vkBeginCommandBuffer().");

		// TODO: команды рендера
		// Начало render pass
		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = m_RenderPass;
		renderPassInfo.framebuffer = m_Framebuffers[frame.imageIndex];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = m_SwapchainExtent;

		VkClearValue clearValues[2];
		clearValues[0].color = { {0.3f, 0.3f, 0.6f, 1.0f} };
		clearValues[1].depthStencil = { 1.0f, 0 };

		renderPassInfo.clearValueCount = 2;
		renderPassInfo.pClearValues = clearValues;

		vkCmdBeginRenderPass(frame.cmdBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		// Здесь ваши команды рендеринга (draw calls)
		vkCmdEndRenderPass(frame.cmdBuffer);

		vr = vkEndCommandBuffer(frame.cmdBuffer);
		if (vr != VK_SUCCESS)
			throw_runtime_error("Failed to vkEndCommandBuffer().");

		frame.inFlight = true;
	}

	void SurfView_VK::RenderFrame()
	{
		static zU32 stepCounter = 0;
		stepCounter++;

		VkDevice device = m_VulkanAPI->GetDevice();
		zU32 indexRender = m_VulkanAPI->GetIndexFrameRender();
		//zU32 indexRender = m_VulkanAPI->GetIndexFramePrepare();
		auto& frame = m_Frames[indexRender];

		DebugOutputLite(L">>>>> RenderFrame(). stepCounter: {}, indexRender: {}.", stepCounter, indexRender);

		VkResult vr = vkResetFences(device, 1, &frame.fence);
		if (vr != VK_SUCCESS)
			throw_runtime_error("Failed to vkResetFences().");

		VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;// VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &frame.acquireSemaphore;
		submitInfo.pWaitDstStageMask = &waitStage;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &frame.cmdBuffer;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &frame.renderSemaphore;
		vr = vkQueueSubmit(m_VulkanAPI->GetGraphicsQueue(), 1, &submitInfo, frame.fence);
		if (vr != VK_SUCCESS)
			throw_runtime_error("Failed to vkQueueSubmit().");
	}

	void SurfView_VK::PostRender()
	{
		static zU32 stepCounter = 0;
		stepCounter++;

		VkDevice device = m_VulkanAPI->GetDevice();
		zU32 indexRender = m_VulkanAPI->GetIndexFrameRender();
		//zU32 indexRender = m_VulkanAPI->GetIndexFramePrepare();
		auto& frame = m_Frames[indexRender];

		DebugOutputLite(L">>>>> PostRender(). stepCounter: {}, indexRender: {}.", stepCounter, indexRender);

		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &frame.renderSemaphore;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &m_Swapchain;
		presentInfo.pImageIndices = &frame.imageIndex;
		std::lock_guard<std::mutex> lock(m_SwapchainMutex);
		VkResult vr = vkQueuePresentKHR(m_VulkanAPI->GetPresentQueue(), &presentInfo);
		if (vr != VK_SUCCESS)
			throw_runtime_error("Failed to vkQueuePresentKHR().");

		frame.inFlight = false;
	}
#pragma endregion

	void SurfView_VK::OnResize(const Size2D<>& size)
	{
		if (m_GAPI->GetInitState() != eInitState::InitOK)
			return;

		if (size.width == 0 || size.height == 0)
			return;

		VkDevice device = m_VulkanAPI->GetDevice();
		ensure(device, "Logical device is not initialized.");

		vkDeviceWaitIdle(device);

		m_SurfSize = size;
		auto res = RecreateSwapchain();
		if (!res)
		{
			auto errorMsg = wstring_to_string(res.error().getMessage());
			throw_runtime_error(std::format("[SurfView_VK::OnResize] {}", errorMsg));
		}
	}

	[[nodiscard]] Result<> SurfView_VK::OnUpdateVSyncState()
	{
		if (m_GAPI->GetInitState() != eInitState::InitOK)
			return {};

		return RecreateSwapchain();
	}
}

#endif // ZRENDER_API_VULKAN