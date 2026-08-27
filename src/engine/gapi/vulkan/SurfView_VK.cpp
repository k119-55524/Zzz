#include "SurfView_VK.h"
#include "Swapchain_VK.h"
#include "DepthBuffer_VK.h"
#include "engine/utils/EngineLogFlags.h"

#if defined(Z_VULKAN)

namespace zzz::engine
{
	SurfView_VK::SurfView_VK(std::shared_ptr<NativeWindow> window, std::shared_ptr<VulkanAPI> gapi)
		: ISurfView(std::move(window), std::move(gapi))
	{
	}

	SurfView_VK::~SurfView_VK()
	{
		OnSurfaceDestroyed();
	}

#pragma region Surface Lifecycle
	void SurfView_VK::OnSurfaceCreated(void* handle)
	{
		DOut("[SurfView_VK::OnSurfaceCreated] - Handle: {}", handle);
		
		auto vkSwapchain = std::make_unique<Swapchain_VK>(m_GAPI, m_Window);
		VkSurfaceKHR surface = CreateVulkanSurface(handle);
		vkSwapchain->Initialize(surface);
		m_Swapchain = std::move(vkSwapchain);

		m_OldSize = m_Swapchain->GetSize();
		m_DepthBuffer = std::make_unique<DepthBuffer_VK>(m_GAPI, m_OldSize);

		m_Swapchain->SetClearConfig(m_ClearConfig.surface);
		m_DepthBuffer->SetClearConfig(m_ClearConfig.depthBuffer);

		Initialize();
	}

	VkSurfaceKHR SurfView_VK::CreateVulkanSurface(void* handle)
	{
		VkSurfaceKHR surface = VK_NULL_HANDLE;
		VkInstance instance = m_GAPI->GetInstance();

#if defined(Z_WINDOWS)
		VkWin32SurfaceCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		createInfo.hinstance = GetModuleHandle(nullptr);
		createInfo.hwnd = static_cast<HWND>(handle);

		VkResult vr = vkCreateWin32SurfaceKHR(instance, &createInfo, nullptr, &surface);
		if (vr != VK_SUCCESS)
			THROW_RUNTIME("[SurfView_VK::CreateVulkanSurface] Failed to create Win32 surface: 0x{:08X}", static_cast<uint32_t>(vr));
#elif defined(Z_ANDROID)
		VkAndroidSurfaceCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
		createInfo.window = static_cast<ANativeWindow*>(handle);

		VkResult vr = vkCreateAndroidSurfaceKHR(instance, &createInfo, nullptr, &surface);
		if (vr != VK_SUCCESS)
			THROW_RUNTIME("[SurfView_VK::CreateVulkanSurface] Failed to create Android surface: 0x{:08X}", static_cast<uint32_t>(vr));
#elif defined(Z_LINUX)
#if defined(USE_WAYLAND)
		VkWaylandSurfaceCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
		createInfo.display = static_cast<wl_display*>(m_Window->GetWaylandDisplay());
		createInfo.surface = static_cast<wl_surface*>(handle);

		VkResult vr = vkCreateWaylandSurfaceKHR(instance, &createInfo, nullptr, &surface);
		if (vr != VK_SUCCESS)
			THROW_RUNTIME("[SurfView_VK::CreateVulkanSurface] Failed to create Wayland surface: 0x{:08X}", static_cast<uint32_t>(vr));
#else
		VkXcbSurfaceCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
		createInfo.connection = static_cast<xcb_connection_t*>(m_Window->GetXcbConnection());
		createInfo.window = static_cast<xcb_window_t>(reinterpret_cast<uintptr_t>(handle));

		VkResult vr = vkCreateXcbSurfaceKHR(instance, &createInfo, nullptr, &surface);
		if (vr != VK_SUCCESS)
			THROW_RUNTIME("[SurfView_VK::CreateVulkanSurface] Failed to create XCB surface: 0x{:08X}", static_cast<uint32_t>(vr));
#endif
#endif
		return surface;
	}

	void SurfView_VK::OnSurfaceDestroyed()
	{
		DOut("[SurfView_VK::OnSurfaceDestroyed]");

		std::lock_guard<std::mutex> lock(m_SubmitMutex);

		if (!m_GAPI)
			return;

		m_GAPI->WaitForGpu();
		VkDevice device = m_GAPI->GetDevice();
		if (!device)
			return;

		for (size_t i = 0; i < c_FramesInFlight; ++i)
		{
			if (m_InFlightFences[i])
			{
				vkDestroyFence(device, m_InFlightFences[i], nullptr);
				m_InFlightFences[i] = VK_NULL_HANDLE;
			}
			if (m_ImageAvailableSemaphores[i])
			{
				vkDestroySemaphore(device, m_ImageAvailableSemaphores[i], nullptr);
				m_ImageAvailableSemaphores[i] = VK_NULL_HANDLE;
			}
		}

		for (auto semaphore : m_RenderFinishedSemaphores)
		{
			if (semaphore)
				vkDestroySemaphore(device, semaphore, nullptr);
		}
		m_RenderFinishedSemaphores.clear();

		if (m_CommandPool)
		{
			vkDestroyCommandPool(device, m_CommandPool, nullptr);
			m_CommandPool = VK_NULL_HANDLE;
		}

		if (m_DepthBuffer)
		{
			m_DepthBuffer->Release();
			m_DepthBuffer.reset();
		}

		if (m_Swapchain)
		{
			auto vkSwapchain = static_cast<Swapchain_VK*>(m_Swapchain.get());
			VkSurfaceKHR surface = vkSwapchain->GetSurface();
			m_Swapchain->Release();
			m_Swapchain.reset();

			if (surface != VK_NULL_HANDLE)
				vkDestroySurfaceKHR(m_GAPI->GetInstance(), surface, nullptr);
		}
	}

	void SurfView_VK::Initialize()
	{
		VkDevice device = m_GAPI->GetDevice();
		ensure(device, "Vulkan Device не должен быть null.");

		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = m_GAPI->GetGraphicsQueueFamilyIndex();

		VkResult vr = vkCreateCommandPool(device, &poolInfo, nullptr, &m_CommandPool);
		if (vr != VK_SUCCESS)
			THROW_RUNTIME("[SurfView_VK::Initialize] Failed to create VkCommandPool: 0x{:08X}", static_cast<uint32_t>(vr));

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = m_CommandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = c_FramesInFlight;

		vr = vkAllocateCommandBuffers(device, &allocInfo, m_CommandBuffers.data());
		if (vr != VK_SUCCESS)
			THROW_RUNTIME("[SurfView_VK::Initialize] Failed to allocate VkCommandBuffers: 0x{:08X}", static_cast<uint32_t>(vr));

		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < c_FramesInFlight; ++i)
		{
			if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &m_ImageAvailableSemaphores[i]) != VK_SUCCESS ||
				vkCreateFence(device, &fenceInfo, nullptr, &m_InFlightFences[i]) != VK_SUCCESS)
			{
				THROW_RUNTIME("[SurfView_VK::Initialize] Failed to create sync objects for frame [{}]", i);
			}
			m_IsRecording[i] = false;
		}

		auto vkSwapchain = static_cast<Swapchain_VK*>(m_Swapchain.get());
		const size_t imageCount = vkSwapchain ? vkSwapchain->GetImageCount() : 3;

		m_RenderFinishedSemaphores.resize(imageCount, VK_NULL_HANDLE);

		for (size_t i = 0; i < imageCount; ++i)
		{
			if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &m_RenderFinishedSemaphores[i]) != VK_SUCCESS)
			{
				THROW_RUNTIME("[SurfView_VK::Initialize] Failed to create VkSemaphore for image [{}]", i);
			}
		}
	}
#pragma endregion // Initialize

	void SurfView_VK::PrepareFrame()
	{
		if (!m_Swapchain || !m_DepthBuffer)
			return;

		if (m_OldSize.GetWidth() == 0 || m_OldSize.GetHeight() == 0)
			return;

		VkDevice device = m_GAPI->GetDevice();
		const uint32_t prepIdx = GetPrepareIndex();

		DOut(0.5, "[SurfView_VK::PrepareFrame] FrameStart prepIdx={}", prepIdx);

		// Ожидание фенса текущего prepIdx (renderIdx — другой индекс, конфликта нет)
		VkResult fenceRes = vkWaitForFences(device, 1, &m_InFlightFences[prepIdx], VK_TRUE, UINT64_MAX);
		if (fenceRes != VK_SUCCESS)
		{
			DOutError("[SurfView_VK::PrepareFrame] vkWaitForFences failed: 0x{:08X}", static_cast<uint32_t>(fenceRes));
			return;
		}
		vkResetFences(device, 1, &m_InFlightFences[prepIdx]);

		auto vkSwapchain = static_cast<Swapchain_VK*>(m_Swapchain.get());
		auto vkDepthBuffer = static_cast<DepthBuffer_VK*>(m_DepthBuffer.get());

		// 1. Запрос следующего изображения у Swapchain
		uint32_t imageIndex = 0;
		VkResult vr = vkAcquireNextImageKHR(device, vkSwapchain->GetSwapchain(), UINT64_MAX, m_ImageAvailableSemaphores[prepIdx], VK_NULL_HANDLE, &imageIndex);
		if (vr == VK_ERROR_OUT_OF_DATE_KHR)
		{
			DOut("[SurfView_VK::PrepareFrame] Swapchain OUT_OF_DATE. Resizing...");
			m_Swapchain->OnResize(m_OldSize);
			return;
		}
		else if (vr != VK_SUCCESS && vr != VK_SUBOPTIMAL_KHR)
		{
			DOutError("[SurfView_VK::PrepareFrame] Failed vkAcquireNextImageKHR: 0x{:08X}", static_cast<uint32_t>(vr));
			return;
		}

		m_CurrentImageIndex[prepIdx] = imageIndex;

		VkCommandBuffer cmd = m_CommandBuffers[prepIdx];
		if (!cmd || m_IsRecording[prepIdx])
			return;

		vkResetCommandBuffer(cmd, 0);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vr = vkBeginCommandBuffer(cmd, &beginInfo);
		if (vr != VK_SUCCESS)
		{
			DOutError("[SurfView_VK::PrepareFrame] Failed to begin Command Buffer [{}]: 0x{:08X}", prepIdx, static_cast<uint32_t>(vr));
			return;
		}

		m_IsRecording[prepIdx] = true;

		// 2. Барьер для Swapchain Image: UNDEFINED -> COLOR_ATTACHMENT
		VkImageMemoryBarrier colorBarrier{};
		colorBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		colorBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		colorBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		colorBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		colorBarrier.image = vkSwapchain->GetBackBuffer(imageIndex);
		colorBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		colorBarrier.subresourceRange.baseMipLevel = 0;
		colorBarrier.subresourceRange.levelCount = 1;
		colorBarrier.subresourceRange.baseArrayLayer = 0;
		colorBarrier.subresourceRange.layerCount = 1;
		colorBarrier.srcAccessMask = 0;
		colorBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		vkCmdPipelineBarrier(cmd,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			0, 0, nullptr, 0, nullptr, 1, &colorBarrier
		);

		// 3. Переход барьера для DepthBuffer при первом использовании: UNDEFINED -> DEPTH_STENCIL_ATTACHMENT_OPTIMAL
		if (!m_IsDepthInitialLayoutTransitioned)
		{
			VkImageMemoryBarrier depthBarrier{};
			depthBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			depthBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			depthBarrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			depthBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			depthBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			depthBarrier.image = vkDepthBuffer->GetImage();
			depthBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			if (vkDepthBuffer->GetFormat() == VK_FORMAT_D24_UNORM_S8_UINT || vkDepthBuffer->GetFormat() == VK_FORMAT_D32_SFLOAT_S8_UINT)
				depthBarrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
			depthBarrier.subresourceRange.baseMipLevel = 0;
			depthBarrier.subresourceRange.levelCount = 1;
			depthBarrier.subresourceRange.baseArrayLayer = 0;
			depthBarrier.subresourceRange.layerCount = 1;
			depthBarrier.srcAccessMask = 0;
			depthBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

			vkCmdPipelineBarrier(cmd,
				VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
				VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
				0, 0, nullptr, 0, nullptr, 1, &depthBarrier
			);
			m_IsDepthInitialLayoutTransitioned = true;
		}

		// Dynamic Rendering Pass (Vulkan 1.3 / 1.4)
		const auto& clearColor = m_ClearConfig.surface.color;
		VkClearValue colorClearValue{};
		colorClearValue.color = { clearColor.R, clearColor.G, clearColor.B, clearColor.A };

		VkRenderingAttachmentInfo colorAttachment{};
		colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
		colorAttachment.imageView = vkSwapchain->GetImageView(imageIndex);
		colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		colorAttachment.loadOp = (m_ClearConfig.surface.mode == eSurfaceClearMode::Color) ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.clearValue = colorClearValue;

		VkClearValue depthClearValue{};
		depthClearValue.depthStencil = { m_ClearConfig.depthBuffer.depth, m_ClearConfig.depthBuffer.stencil };

		VkRenderingAttachmentInfo depthAttachment{};
		depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
		depthAttachment.imageView = vkDepthBuffer->GetImageView();
		depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		depthAttachment.loadOp = (m_ClearConfig.depthBuffer.depthMode == eClearDepthMode::Depth) ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		depthAttachment.clearValue = depthClearValue;

		VkRenderingInfo renderingInfo{};
		renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
		renderingInfo.renderArea = { {0, 0}, {static_cast<uint32_t>(m_OldSize.GetWidth()), static_cast<uint32_t>(m_OldSize.GetHeight())} };
		renderingInfo.layerCount = 1;
		renderingInfo.colorAttachmentCount = 1;
		renderingInfo.pColorAttachments = &colorAttachment;
		renderingInfo.pDepthAttachment = &depthAttachment;

		vkCmdBeginRendering(cmd, &renderingInfo);
		// Clear and draw operations
		vkCmdEndRendering(cmd);

		// Transition Swapchain Image: COLOR_ATTACHMENT -> PRESENT_SRC
		colorBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		colorBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		colorBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		colorBarrier.dstAccessMask = 0;

		vkCmdPipelineBarrier(cmd,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
			0, 0, nullptr, 0, nullptr, 1, &colorBarrier
		);

		vkEndCommandBuffer(cmd);
	}

	void SurfView_VK::RenderFrame()
	{
		if (!m_Swapchain)
			return;

		const uint32_t renderIdx = GetRenderIndex();
		VkCommandBuffer cmd = m_CommandBuffers[renderIdx];
		if (!cmd || !m_IsRecording[renderIdx])
			return;

		const uint32_t imgIdx = m_CurrentImageIndex[renderIdx];
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &m_ImageAvailableSemaphores[renderIdx];
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmd;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &m_RenderFinishedSemaphores[imgIdx];

		DOut(0.5, "[SurfView_VK::RenderFrame] Submit&Present renderIdx={} imgIdx={}", renderIdx, imgIdx);

		VkResult vr = VK_SUCCESS;
		{
			std::lock_guard lock(m_SubmitMutex);
			vr = vkQueueSubmit(m_GAPI->GetGraphicsQueue(), 1, &submitInfo, m_InFlightFences[renderIdx]);
		}

		if (vr != VK_SUCCESS)
		{
			DOutError("[SurfView_VK::RenderFrame] Failed vkQueueSubmit: 0x{:08X}", static_cast<uint32_t>(vr));
			return;
		}

		auto vkSwapchain = static_cast<Swapchain_VK*>(m_Swapchain.get());
		vkSwapchain->Present(m_GAPI->IsCanDisableVSync(), imgIdx, m_RenderFinishedSemaphores[imgIdx]);

		m_IsRecording[renderIdx] = false;
	}

	void SurfView_VK::OnResize(const Size2D<>& size)
	{
		if (!m_Swapchain || !m_DepthBuffer)
			return;

		std::lock_guard<std::mutex> lock(m_SubmitMutex);

		m_GAPI->WaitForGpu();
		m_OldSize = size;
		m_Swapchain->OnResize(size);
		m_DepthBuffer->OnResize(size);
	}
}

#endif // Z_VULKAN
