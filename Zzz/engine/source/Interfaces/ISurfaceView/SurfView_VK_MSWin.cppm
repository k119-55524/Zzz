export module SurfView_VK_MSWin;

#if defined(ZRENDER_API_VULKAN)

import Math;
import IPSO;
import IGAPI;
import VKAPI;
import Scene;
import Result;
import Size2D;
import Colors;
import Camera;
import IAppWin;
import Helpers;
import IMeshGPU;
import ISurfView;
import StrConvert;
import RenderQueue;
import RenderVolume;
import ViewportDesc;
import AppWin_MSWin;
import EngineConstants;
import PrimitiveTopology;

using namespace zzz::math;
using namespace zzz::core;
using namespace zzz::colors;

namespace zzz::vk
{
	// Структуры для uniform buffers (аналог UploadBuffer в DirectX)
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

	export class SurfView_VK_MSWin final : public ISurfView
	{
		Z_NO_COPY_MOVE(SurfView_VK_MSWin);

	public:
		explicit SurfView_VK_MSWin(std::shared_ptr<IAppWin> _iAppWin, std::shared_ptr<IGAPI> _iGAPI);
		~SurfView_VK_MSWin() override;

		[[nodiscard]] Result<> Initialize() override;
		void PrepareFrame(const std::shared_ptr<RenderQueue> renderQueue) override;
		void RenderFrame() override;
		void OnResize(const Size2D<>& size) override;
		void SetFullScreen(bool fs) override;

	private:
		std::shared_ptr<AppWin_MSWin> m_iAppWin;
		std::shared_ptr<VKAPI> m_VulkanAPI;

		static constexpr VkFormat BACK_BUFFER_FORMAT = VK_FORMAT_B8G8R8A8_UNORM;
		static constexpr VkFormat DEPTH_FORMAT = VK_FORMAT_D32_SFLOAT;

		VkSurfaceKHR m_Surface;
		VkSwapchainKHR m_Swapchain;
		VkExtent2D m_SwapchainExtent;
		std::vector<VkImage> m_SwapchainImages;
		std::vector<VkImageView> m_SwapchainImageViews;
		std::vector<VkFramebuffer> m_Framebuffers;

		VkImage m_DepthImage;
		VkDeviceMemory m_DepthImageMemory;
		VkImageView m_DepthImageView;
		VkRenderPass m_RenderPass;

		[[nodiscard]] Result<> CreateSurface();
		[[nodiscard]] Result<> CreateSwapchain(const Size2D<>& size);
		[[nodiscard]] Result<> CreateImageViews();
		[[nodiscard]] Result<> CreateRenderPass();
		[[nodiscard]] Result<> CreateDepthResources(const Size2D<>& size);
		[[nodiscard]] Result<> CreateFramebuffers();
		[[nodiscard]] Result<> RecreateSwapchain(const Size2D<>& size);
		[[nodiscard]] Result<> CreateSyncObjects();
		void CleanupSwapchain();
		[[nodiscard]] uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

		VkSemaphore m_StagingAcquireSemaphore;
		VkSemaphore m_StagingRenderFinishedSemaphore;
		std::array<VkSemaphore, BACK_BUFFER_COUNT> m_AcquireSemaphores{};
		std::array<VkSemaphore, BACK_BUFFER_COUNT> m_RenderFinishedSemaphores{};

		std::mutex m_FrameMutex;
		std::condition_variable m_FrameReady;
		bool m_IsFramePrepared = false;
		uint32_t m_PreparedImageIndex = 0;
	};

	SurfView_VK_MSWin::SurfView_VK_MSWin(
		std::shared_ptr<IAppWin> _iAppWin,
		std::shared_ptr<IGAPI> _iGAPI)
		: ISurfView(_iGAPI),
		m_Surface{ VK_NULL_HANDLE },
		m_Swapchain{ VK_NULL_HANDLE },
		m_DepthImage{ VK_NULL_HANDLE },
		m_DepthImageMemory{ VK_NULL_HANDLE },
		m_DepthImageView{ VK_NULL_HANDLE },
		m_RenderPass{ VK_NULL_HANDLE },
		m_StagingAcquireSemaphore{ VK_NULL_HANDLE },
		m_StagingRenderFinishedSemaphore{VK_NULL_HANDLE}
	{
		ensure(_iAppWin, "App window cannot be null.");
		m_iAppWin = std::dynamic_pointer_cast<AppWin_MSWin>(_iAppWin);
		ensure(m_iAppWin, "App window must be of type AppWin_MSWin.");

		m_VulkanAPI = std::dynamic_pointer_cast<VKAPI>(_iGAPI);
		ensure(m_VulkanAPI, "Failed to cast IGAPI to VKAPI.");
	}

	SurfView_VK_MSWin::~SurfView_VK_MSWin()
	{
		VkDevice device = m_VulkanAPI->GetDevice();
		if (!device)
			return;

		vkDeviceWaitIdle(device);

		if (m_StagingAcquireSemaphore)
		{
			vkDestroySemaphore(device, m_StagingAcquireSemaphore, nullptr);
			m_StagingAcquireSemaphore = VK_NULL_HANDLE;
		}

		if (m_StagingRenderFinishedSemaphore)
		{
			vkDestroySemaphore(device, m_StagingRenderFinishedSemaphore, nullptr);
			m_StagingRenderFinishedSemaphore = VK_NULL_HANDLE;
		}

		for (size_t i = 0; i < BACK_BUFFER_COUNT; i++)
		{
			if (m_AcquireSemaphores[i])
			{
				vkDestroySemaphore(device, m_AcquireSemaphores[i], nullptr);
				m_AcquireSemaphores[i] = VK_NULL_HANDLE;
			}

			if (m_RenderFinishedSemaphores[i])
			{
				vkDestroySemaphore(device, m_RenderFinishedSemaphores[i], nullptr);
				m_RenderFinishedSemaphores[i] = VK_NULL_HANDLE;
			}
		}

		CleanupSwapchain();

		if (m_RenderPass)
			vkDestroyRenderPass(device, m_RenderPass, nullptr);

		if (m_Surface)
			vkDestroySurfaceKHR(m_VulkanAPI->GetInstance(), m_Surface, nullptr);
	}

#pragma region Initialize
	Result<> SurfView_VK_MSWin::Initialize()
	{
		auto winSize = m_iAppWin->GetWinSize();
		m_SurfSize.SetFrom(winSize);

		auto res = CreateSurface()
			.and_then([&]() { return CreateSwapchain(winSize); })
			.and_then([&]() { return CreateImageViews(); })
			.and_then([&]() { return CreateRenderPass(); })
			.and_then([&]() { return CreateDepthResources(winSize); })
			.and_then([&]() { return CreateFramebuffers(); })
			.and_then([&]() { return CreateSyncObjects(); });

		if (!res)
			return Unexpected(eResult::failure, std::format(L"Failed to initialize SurfView: {}", res.error().getMessage()));

		return {};
	}

	[[nodiscard]] Result<> SurfView_VK_MSWin::CreateSurface()
	{
		VkWin32SurfaceCreateInfoKHR createInfo{
			.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
			.hinstance = GetModuleHandle(nullptr),
			.hwnd = m_iAppWin->GetHWND()
		};

		VkResult vr = vkCreateWin32SurfaceKHR(m_VulkanAPI->GetInstance(), &createInfo, nullptr, &m_Surface);
		if (vr != VK_SUCCESS)
			return Unexpected(eResult::failure, std::format(L"Failed to create surface ({})", int(vr)));

		return {};
	}

	[[nodiscard]] Result<> SurfView_VK_MSWin::CreateSwapchain(const Size2D<>& size)
	{
		VkPhysicalDevice physicalDevice = m_VulkanAPI->GetPhysicalDevice();
		VkDevice device = m_VulkanAPI->GetDevice();
		ensure(physicalDevice, "Physical device is not initialized.");
		ensure(device, "Logical device is not initialized.");

		VkSurfaceCapabilitiesKHR capabilities;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, m_Surface, &capabilities);

		// Вычисляем корректный extent
		VkExtent2D extent;
		if (capabilities.currentExtent.width != UINT32_MAX)
		{
			extent = capabilities.currentExtent;
		}
		else
		{
			extent =
			{
				std::clamp(static_cast<uint32_t>(size.width), capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
				std::clamp(static_cast<uint32_t>(size.height), capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
			};
		}

		m_SwapchainExtent = extent;

		// Далее создаём swapchain с этим extent
		uint32_t imageCount = BACK_BUFFER_COUNT;
		if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount)
			imageCount = capabilities.maxImageCount;

		VkSwapchainCreateInfoKHR createInfo{
			.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
			.surface = m_Surface,
			.minImageCount = imageCount,
			.imageFormat = BACK_BUFFER_FORMAT,
			.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
			.imageExtent = extent,
			.imageArrayLayers = 1,
			.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.preTransform = capabilities.currentTransform,
			.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
			.presentMode = m_iGAPI->IsCanDisableVsync() ? VK_PRESENT_MODE_IMMEDIATE_KHR : VK_PRESENT_MODE_FIFO_KHR,
			.clipped = VK_TRUE,
			.oldSwapchain = VK_NULL_HANDLE
		};

		VkResult vr = vkCreateSwapchainKHR(device, &createInfo, nullptr, &m_Swapchain);
		if (vr != VK_SUCCESS)
			return Unexpected(eResult::failure, std::format(L"Failed to create swapchain ({})", int(vr)));

		// Получаем изображения swapchain
		vkGetSwapchainImagesKHR(device, m_Swapchain, &imageCount, nullptr);
		m_SwapchainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(device, m_Swapchain, &imageCount, m_SwapchainImages.data());

		return {};
	}

	[[nodiscard]] Result<> SurfView_VK_MSWin::CreateImageViews()
	{
		VkDevice device = m_VulkanAPI->GetDevice();
		ensure(device, "Logical device is not initialized.");

		m_SwapchainImageViews.resize(m_SwapchainImages.size());

		for (size_t i = 0; i < m_SwapchainImages.size(); i++)
		{
			VkImageViewCreateInfo createInfo{
				.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
				.image = m_SwapchainImages[i],
				.viewType = VK_IMAGE_VIEW_TYPE_2D,
				.format = BACK_BUFFER_FORMAT,
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
				return Unexpected(eResult::failure, std::format(L"Failed to create image view {} ({})", i, int(vr)));
		}

		return {};
	}

	[[nodiscard]] Result<> SurfView_VK_MSWin::CreateRenderPass()
	{
		VkDevice device = m_VulkanAPI->GetDevice();

		VkAttachmentDescription colorAttachment{
			.format = BACK_BUFFER_FORMAT,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
		};

		VkAttachmentDescription depthAttachment{
			.format = DEPTH_FORMAT,
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
			.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
			.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
			.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT
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
			return Unexpected(eResult::failure, std::format(L"Failed to create render pass ({})", int(vr)));

		return {};
	}

	[[nodiscard]] Result<> SurfView_VK_MSWin::CreateDepthResources(const Size2D<>& size)
	{
		VkDevice device = m_VulkanAPI->GetDevice();
		VkPhysicalDevice physicalDevice = m_VulkanAPI->GetPhysicalDevice();

		VkImageCreateInfo imageInfo{
			.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			.imageType = VK_IMAGE_TYPE_2D,
			.format = DEPTH_FORMAT,
			.extent = {
				m_SwapchainExtent.width,	//.width = static_cast<uint32_t>(size.width),
				m_SwapchainExtent.height,	//.height = static_cast<uint32_t>(size.height),
				1
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
			return Unexpected(eResult::failure, std::format(L"Failed to create depth image ({})", int(vr)));

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(device, m_DepthImage, &memRequirements);

		VkMemoryAllocateInfo allocInfo{
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.allocationSize = memRequirements.size,
			.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
		};

		vr = vkAllocateMemory(device, &allocInfo, nullptr, &m_DepthImageMemory);
		if (vr != VK_SUCCESS)
			return Unexpected(eResult::failure, std::format(L"Failed to allocate depth image memory ({})", int(vr)));

		vkBindImageMemory(device, m_DepthImage, m_DepthImageMemory, 0);

		VkImageViewCreateInfo viewInfo{
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = m_DepthImage,
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = DEPTH_FORMAT,
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
			return Unexpected(eResult::failure, std::format(L"Failed to create depth image view ({})", int(vr)));

		return {};
	}

	[[nodiscard]] Result<> SurfView_VK_MSWin::CreateFramebuffers()
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
				return Unexpected(eResult::failure, std::format(L"Failed to create framebuffer {} ({})", i, int(vr)));
		}

		return {};
	}

	Result<> SurfView_VK_MSWin::CreateSyncObjects()
	{
		VkDevice device = m_VulkanAPI->GetDevice();
		VkSemaphoreCreateInfo semInfo{ .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };

		if (vkCreateSemaphore(device, &semInfo, nullptr, &m_StagingAcquireSemaphore) != VK_SUCCESS)
			return Unexpected(eResult::failure, L"Failed to create StagingAcquireSemaphore");

		if (vkCreateSemaphore(device, &semInfo, nullptr, &m_StagingRenderFinishedSemaphore) != VK_SUCCESS)
			return Unexpected(eResult::failure, L"Failed to create StagingRenderFinishedSemaphore");

		for (size_t i = 0; i < BACK_BUFFER_COUNT; i++)
		{
			if (vkCreateSemaphore(device, &semInfo, nullptr, &m_AcquireSemaphores[i]) != VK_SUCCESS)
				return Unexpected(eResult::failure, std::format(L"Failed to create AcquireSemaphore [{}]", i));

			// per-image — остаётся в SurfView
			if (vkCreateSemaphore(device, &semInfo, nullptr, &m_RenderFinishedSemaphores[i]) != VK_SUCCESS)
				return Unexpected(eResult::failure, std::format(L"Failed to create RenderFinishedSemaphore [{}]", i));
		}

		return {};
	}

	void SurfView_VK_MSWin::CleanupSwapchain()
	{
		VkDevice device = m_VulkanAPI->GetDevice();

		for (auto framebuffer : m_Framebuffers)
			vkDestroyFramebuffer(device, framebuffer, nullptr);

		if (m_DepthImageView)
			vkDestroyImageView(device, m_DepthImageView, nullptr);
		if (m_DepthImage)
			vkDestroyImage(device, m_DepthImage, nullptr);
		if (m_DepthImageMemory)
			vkFreeMemory(device, m_DepthImageMemory, nullptr);

		for (auto imageView : m_SwapchainImageViews)
			vkDestroyImageView(device, imageView, nullptr);

		if (m_Swapchain)
			vkDestroySwapchainKHR(device, m_Swapchain, nullptr);
	}

	[[nodiscard]] uint32_t SurfView_VK_MSWin::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(m_VulkanAPI->GetPhysicalDevice(), &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
		{
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
				return i;
		}

		throw_runtime_error("Failed to find suitable memory type");
	}

	[[nodiscard]] Result<> SurfView_VK_MSWin::RecreateSwapchain(const Size2D<>& size)
	{
		VkDevice device = m_VulkanAPI->GetDevice();
		ensure(device, "Logical device is not initialized.");

		vkDeviceWaitIdle(device);

		// Удаляем старые ресурсы swapchain
		CleanupSwapchain();

		// Получаем capabilities поверхности
		VkSurfaceCapabilitiesKHR capabilities;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_VulkanAPI->GetPhysicalDevice(), m_Surface, &capabilities);

		// Вычисляем новый extent swapchain
		if (capabilities.currentExtent.width != UINT32_MAX)
		{
			m_SwapchainExtent = capabilities.currentExtent;
		}
		else
		{
			m_SwapchainExtent =
			{
				std::clamp(static_cast<uint32_t>(size.width), capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
				std::clamp(static_cast<uint32_t>(size.height), capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
			};
		}

		// Создаём swapchain с новым extent
		auto res = CreateSwapchain(Size2D<>(m_SwapchainExtent.width, m_SwapchainExtent.height))
			.and_then([&]() { return CreateImageViews(); })
			.and_then([&]() { return CreateDepthResources(Size2D<>(m_SwapchainExtent.width, m_SwapchainExtent.height)); })
			.and_then([&]() { return CreateFramebuffers(); });

		if (!res)
			return Unexpected(eResult::failure, std::format(L"Failed to recreate swapchain: {}", res.error().getMessage()));

		return {};
	}
#pragma endregion Initialize

#pragma region Rendering
	void SurfView_VK_MSWin::PrepareFrame(const std::shared_ptr<RenderQueue> renderQueue)
	{
		VkDevice device = m_VulkanAPI->GetDevice();

		uint32_t imageIndex;
		VkResult vr = vkAcquireNextImageKHR(
			device, m_Swapchain, UINT64_MAX,
			m_StagingAcquireSemaphore, // остаётся здесь — привязан к swapchain
			VK_NULL_HANDLE, &imageIndex
		);

		if (vr == VK_ERROR_OUT_OF_DATE_KHR)
		{
			auto res = RecreateSwapchain(m_SurfSize);
			if (!res)
				throw_runtime_error("Failed to recreate swapchain");

			std::lock_guard lock(m_FrameMutex);
			m_IsFramePrepared = false;
			m_FrameReady.notify_one();
			return;
		}
		if (vr != VK_SUCCESS && vr != VK_SUBOPTIMAL_KHR)
			throw_runtime_error("vkAcquireNextImageKHR failed");

		std::swap(m_StagingAcquireSemaphore, m_AcquireSemaphores[imageIndex]);

		VkCommandBuffer cmd = m_VulkanAPI->GetGraphicsCBUpdate();
		VkClearValue clearValues[2]{};
		clearValues[0].color = { 0.5f, 0.5f, 0.5f, 1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };

		VkRenderPassBeginInfo rpInfo{
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
			.renderPass = m_RenderPass,
			.framebuffer = m_Framebuffers[imageIndex],
			.renderArea = {.offset = {0,0}, .extent = m_SwapchainExtent },
			.clearValueCount = 2,
			.pClearValues = clearValues
		};

		vkCmdBeginRenderPass(cmd, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdEndRenderPass(cmd);
		{
			std::lock_guard lock(m_FrameMutex);
			m_PreparedImageIndex = imageIndex;
			m_IsFramePrepared = true;
		}
		m_FrameReady.notify_one();
	}

	void SurfView_VK_MSWin::RenderFrame()
	{
		uint32_t imageIndex;
		{
			std::unique_lock lock(m_FrameMutex);
			m_FrameReady.wait(lock, [this] { return m_IsFramePrepared; });
			if (!m_IsFramePrepared) return;
			imageIndex = m_PreparedImageIndex;
			m_IsFramePrepared = false;
		}

		// swap RenderFinished — симметрично acquire
		std::swap(m_StagingRenderFinishedSemaphore, m_RenderFinishedSemaphores[imageIndex]);

		VkSemaphore waitSem = m_AcquireSemaphores[imageIndex];
		VkSemaphore signalSem = m_RenderFinishedSemaphores[imageIndex];

		m_VulkanAPI->SetFrameSyncData(waitSem, signalSem);
		m_VulkanAPI->SubmitCommandLists();

		VkPresentInfoKHR presentInfo{
			.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &signalSem,
			.swapchainCount = 1,
			.pSwapchains = &m_Swapchain,
			.pImageIndices = &imageIndex
		};

		VkResult vr = vkQueuePresentKHR(m_VulkanAPI->GetGraphicsQueue(), &presentInfo);
		if (vr == VK_ERROR_OUT_OF_DATE_KHR || vr == VK_SUBOPTIMAL_KHR)
		{
			if (auto res = RecreateSwapchain(m_SurfSize); !res)
				throw_runtime_error("Failed to recreate swapchain");
		}
		else if (vr != VK_SUCCESS)
			throw_runtime_error("vkQueuePresentKHR failed");
	}
#pragma endregion Rendering

	void SurfView_VK_MSWin::OnResize(const Size2D<>& size)
	{
		if (m_iGAPI->GetInitState() != eInitState::InitOK)
			return;

		if (size.width == 0 || size.height == 0)
			return;

		VkDevice device = m_VulkanAPI->GetDevice();
		ensure(device, "Logical device is not initialized.");

		vkDeviceWaitIdle(device);

		auto res = RecreateSwapchain(size);
		if (!res)
			throw_runtime_error(std::format("[SurfView_VK_MSWin::OnResize] {}", wstring_to_string(res.error().getMessage())));

		m_SurfSize = size;
	}

	void SurfView_VK_MSWin::SetFullScreen(bool fs)
	{
		// TODO: Implement fullscreen switching for Vulkan
		DebugOutput(std::format(L"[SurfView_VK_MSWin::SetFullScreen({})] Not implemented yet\n", fs).c_str());
	}
}
#endif // ZRENDER_API_VULKAN