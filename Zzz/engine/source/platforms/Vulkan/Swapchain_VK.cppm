
export module Swapchain_VK;

#if defined(ZRENDER_API_VULKAN)

import VKAPI;
import Result;
import Ensure;
import EngineConstants;

using namespace zzz;

namespace zzz::vk
{
	export class Swapchain_VK final
	{
		Z_NO_COPY_MOVE(Swapchain_VK);

	public:
		Swapchain_VK();
		~Swapchain_VK();

		Result<VkExtent2D> Initialize(std::shared_ptr<VKAPI> gapi, VkSurfaceKHR m_Surface);

	private:
		VkSurfaceFormat2KHR SelectSwapSurfaceFormat(const std::vector<VkSurfaceFormat2KHR>& availableFormats) const;
		VkPresentModeKHR SelectSwapPresentMode(bool vSync, const std::vector<VkPresentModeKHR>& availablePresentModes);
		void CmdTransitionSwapchainLayout(VkCommandBuffer cmd, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout);

		// Представляет собой изображение в цепочке обменов, на которое может быть выполнена отрисовка.
		struct Image
		{
			VkImage		image{ VK_NULL_HANDLE };		// Изображение для рендеринга
			VkImageView	imageView{ VK_NULL_HANDLE };	// Для просмотра изображения перейдите в режим просмотра изображения.
		};

		// Ресурсы, связанные с каждым обрабатываемым кадром.
		// Каждый кадр имеет свой собственный набор ресурсов, в основном примитивов синхронизации.
		struct FrameResources
		{
			VkSemaphore imageAvailableSemaphore{ VK_NULL_HANDLE };	// Сигнализирует о готовности изображения к рендерингу.
			VkSemaphore renderFinishedSemaphore{ VK_NULL_HANDLE };	// Сигнал о завершении рендеринга
		};

		VkDevice m_Device{ VK_NULL_HANDLE };
		VkSwapchainKHR m_SwapChain{ VK_NULL_HANDLE };
		VkFormat m_ImageFormat{}; // Формат изображений свопчейна

		std::vector<Image> m_NextImages;
		std::vector<FrameResources> m_FrameResources;

		uint32_t m_MaxFramesInFlight = FRAMES_IN_FLIGHT; // Лучший вариант практически для всех случаев == 3.
	};

	Swapchain_VK::Swapchain_VK() :
		m_NextImages(FRAMES_IN_FLIGHT),
		m_FrameResources(FRAMES_IN_FLIGHT)
	{};

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
	};

	Result<VkExtent2D> Swapchain_VK::Initialize(std::shared_ptr<VKAPI> gapi, VkSurfaceKHR surface)
	{
		ensure(gapi, "VKAPI cannot be null.");
		VkPhysicalDevice m_PhysicalDevice = gapi->GetPhysicalDevice();
		ensure(m_PhysicalDevice, "Physical device cannot be null.");
		m_Device = gapi->GetDevice();
		ensure(m_Device, "Device cannot be null.");
		ensure(surface, "Surface cannot be null.");
		VkExtent2D winSize;

		// --------------------------------------------------------------------
		// Получение возможностей поверхности (Surface Capabilities)
		// --------------------------------------------------------------------
		// VkPhysicalDeviceSurfaceInfo2KHR — расширенная структура запроса
		// информации о поверхности (используется в API версии *2KHR).
		// Здесь указывается поверхность, для которой мы хотим получить данные.
		const VkPhysicalDeviceSurfaceInfo2KHR surfaceInfo2
		{
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR,
			.surface = surface
		};

		// VkSurfaceCapabilities2KHR содержит информацию:
		//  - минимальное и максимальное количество изображений swapchain
		//  - поддерживаемые размеры изображения
		//  - поддерживаемые трансформации поверхности
		//  - usage flags
		VkSurfaceCapabilities2KHR capabilities2{ .sType = VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_2_KHR };

		// Запрашиваем возможности поверхности для данного GPU
		VkResult vr = vkGetPhysicalDeviceSurfaceCapabilities2KHR(m_PhysicalDevice, &surfaceInfo2, &capabilities2);
		if (vr != VK_SUCCESS)
			return UNEXPECTED(eResult::failure, L"Failed to vkGetPhysicalDeviceSurfaceCapabilities2KHR: {}.", static_cast<int>(vr));

		// --------------------------------------------------------------------
		// Получение списка поддерживаемых форматов surface
		// --------------------------------------------------------------------
		// Первый вызов — только для получения количества форматов.
		uint32_t formatCount = 0;
		vr = vkGetPhysicalDeviceSurfaceFormats2KHR(m_PhysicalDevice, &surfaceInfo2, &formatCount, nullptr);
		if (vr != VK_SUCCESS)
			return UNEXPECTED(eResult::failure, L"Failed to vkGetPhysicalDeviceSurfaceFormats2KHR: {}.", static_cast<int>(vr));

		// Выделяем массив структур VkSurfaceFormat2KHR.
		// Каждый элемент описывает:
		//   - VkFormat (например VK_FORMAT_B8G8R8A8_SRGB)
		//   - VkColorSpaceKHR (обычно VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		std::vector<VkSurfaceFormat2KHR> formats(formatCount, { .sType = VK_STRUCTURE_TYPE_SURFACE_FORMAT_2_KHR });
		vr = vkGetPhysicalDeviceSurfaceFormats2KHR(m_PhysicalDevice, &surfaceInfo2, &formatCount, formats.data());
		if (vr != VK_SUCCESS)
			return UNEXPECTED(eResult::failure, L"Failed to vkGetPhysicalDeviceSurfaceFormats2KHR: {}.", static_cast<int>(vr));

		// --------------------------------------------------------------------
		// Получение поддерживаемых режимов презентации (Present Modes)
		// --------------------------------------------------------------------
		// Present mode определяет стратегию отображения кадров:
		//
		// VK_PRESENT_MODE_FIFO_KHR       — VSync (гарантированно поддерживается)
		// VK_PRESENT_MODE_MAILBOX_KHR    — низкая задержка, triple buffering
		// VK_PRESENT_MODE_IMMEDIATE_KHR  — без VSync, возможен tearing
		// --------------------------------------------------------------------
		// Первый вызов — получаем количество режимов.
		uint32_t presentModeCount = 0;
		vr = vkGetPhysicalDeviceSurfacePresentModesKHR(m_PhysicalDevice, surface, &presentModeCount, nullptr);
		if (vr != VK_SUCCESS)
			return UNEXPECTED(eResult::failure, L"Failed to vkGetPhysicalDeviceSurfacePresentModesKHR: {}.", static_cast<int>(vr));

		// Второй вызов — получаем сами режимы.
		std::vector<VkPresentModeKHR> presentModes(presentModeCount);
		vr = vkGetPhysicalDeviceSurfacePresentModesKHR(m_PhysicalDevice, surface, &presentModeCount, presentModes.data());
		if (vr != VK_SUCCESS)
			return UNEXPECTED(eResult::failure, L"Failed to vkGetPhysicalDeviceSurfacePresentModesKHR: {}.", static_cast<int>(vr));

		// Выберите наиболее подходящий формат поверхности и режим презентации.
		const VkSurfaceFormat2KHR surfaceFormat2 = SelectSwapSurfaceFormat(formats);
		const VkPresentModeKHR presentMode = SelectSwapPresentMode(gapi->IsVSyncEnabled(), presentModes);

		// Запоминаем размер окна
		winSize = capabilities2.surfaceCapabilities.currentExtent;

		// Отрегулируйте количество изображений в процессе передачи в пределах возможностей графического процессора.
		uint32_t minImageCount = capabilities2.surfaceCapabilities.minImageCount; // Минимум, определенный Vulkan
		uint32_t preferredImageCount = std::max(3u, minImageCount); // Предпочтительно 3, но соблюдайте minImageCount.

		// Обработайте случай с параметром maxImageCount, где 0 означает «нет верхнего предела».
		uint32_t maxImageCount = (capabilities2.surfaceCapabilities.maxImageCount == 0) ?
			preferredImageCount :  // Верхнего предела нет, используйте предпочтительный вариант.
			capabilities2.surfaceCapabilities.maxImageCount;

		// Ограничьте значение preferredImageCount допустимым диапазоном [minImageCount, maxImageCount]
		m_MaxFramesInFlight = std::clamp(preferredImageCount, minImageCount, maxImageCount);

		// Сохраняем выбранный формат изображения.
		m_ImageFormat = surfaceFormat2.surfaceFormat.format;

		// Создайте цепочку обменов.
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
			return UNEXPECTED(eResult::failure, L"Failed to vkCreateSwapchainKHR: {}.", static_cast<int>(vr));
		gapi->SetDebugName(m_SwapChain, "m_SwapChain");

		// Получаем изображения swapchain
		{
			uint32_t imageCount = 0;
			vkGetSwapchainImagesKHR(m_Device, m_SwapChain, &imageCount, nullptr);
			ensure(m_MaxFramesInFlight <= imageCount, "Wrong swapchain setup");
			m_MaxFramesInFlight = imageCount; // Использовать число изображений в swapchain
		}
		std::vector<VkImage> swapImages(m_MaxFramesInFlight);
		vr = vkGetSwapchainImagesKHR(m_Device, m_SwapChain, &m_MaxFramesInFlight, swapImages.data());
		if (vr != VK_SUCCESS)
			return UNEXPECTED(eResult::failure, L"Failed to vkGetSwapchainImagesKHR: {}.", static_cast<int>(vr));

		// Сохранить изображения swapchain и создать для них image views
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
			gapi->SetDebugName(m_NextImages[i].image, std::format("VkImage({})", i).c_str());

			imageViewCreateInfo.image = m_NextImages[i].image;
			vr = vkCreateImageView(m_Device, &imageViewCreateInfo, nullptr, &m_NextImages[i].imageView);
			if (vr != VK_SUCCESS)
				return UNEXPECTED(eResult::failure, L"Failed to vkCreateImageView: {}.", static_cast<int>(vr));

			gapi->SetDebugName(m_NextImages[i].image, std::format("VkImageView({})", i).c_str());
		}

		// Инициализировать ресурсы кадра для каждого кадра
		m_FrameResources.resize(m_MaxFramesInFlight);
		for (size_t i = 0; i < m_MaxFramesInFlight; ++i)
		{
			// Объекты синхронизации используются для координации рендеринга и вывода на экран.
			// Семафор "image available" сигнализирует о том, что изображение готово для рендеринга.
			// Семафор "render finished" сигнализирует о том, что рендеринг завершён.
			// Фенс "in flight" сигнализирует о том, что кадр находится в процессе обработки.
			const VkSemaphoreCreateInfo semaphoreCreateInfo{ .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
			vr = vkCreateSemaphore(m_Device, &semaphoreCreateInfo, nullptr, &m_FrameResources[i].imageAvailableSemaphore);
			if (vr != VK_SUCCESS)
				return UNEXPECTED(eResult::failure, L"Failed to vkCreateSemaphore(Available): {}.", static_cast<int>(vr));

			gapi->SetDebugName(m_FrameResources[i].imageAvailableSemaphore, std::format("Available semaphore({})", i).c_str());

			vr = vkCreateSemaphore(m_Device, &semaphoreCreateInfo, nullptr, &m_FrameResources[i].renderFinishedSemaphore);
			if (vr != VK_SUCCESS)
				return UNEXPECTED(eResult::failure, L"Failed to vkCreateImageView(Finished): {}.", static_cast<int>(vr));

			gapi->SetDebugName(m_FrameResources[i].renderFinishedSemaphore, std::format("Render finished semaphore({})", i).c_str());
		}

		// Перевести изображения в layout для презентации
		{
			auto resBegin = gapi->BeginSingleTimeCommands();
			if (resBegin)
			{
				VkCommandBuffer cmd = resBegin.value();
				for (uint32_t i = 0; i < m_MaxFramesInFlight; i++)
				{
					CmdTransitionSwapchainLayout(cmd, m_NextImages[i].image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
				}

				auto resEnd = gapi->EndSingleTimeCommands(cmd);
				if (!resEnd)
					return resEnd.error();
			}
			else
				return resBegin.error();
		}

		return winSize;
	}

	// Мы выбираем наиболее распространенный формат, который поддерживается физическим устройством.
	VkSurfaceFormat2KHR Swapchain_VK::SelectSwapSurfaceFormat(const std::vector<VkSurfaceFormat2KHR>& availableFormats) const
	{
		// Если доступен только один формат, и он не определен, вернуть формат по умолчанию.
		if (availableFormats.size() == 1 && availableFormats[0].surfaceFormat.format == VK_FORMAT_UNDEFINED)
		{
			VkSurfaceFormat2KHR result{
				.sType = VK_STRUCTURE_TYPE_SURFACE_FORMAT_2_KHR,
				.surfaceFormat = { VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR} };
			return result;
		}

		// Сравните доступные форматы с предпочтительными.
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

		// Если ни один из предпочтительных форматов недоступен, вернуть первый доступный формат.
		return availableFormats[0];
	}

	// Текущий режим выбирается на основе параметра vSync.
	// Режим FIFO является наиболее распространенным и используется, когда vSync включен.
	// Режим MAILBOX используется, когда vSync отключен, и является наилучшим режимом для тройной буферизации.
	// Режим IMMEDIATE используется, когда vSync отключен, и является наилучшим режимом для низкой задержки.
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
			return VK_PRESENT_MODE_IMMEDIATE_KHR; // Лучший режим c низкой задержкой

		return VK_PRESENT_MODE_FIFO_KHR;  // Если ни MAILBOX, ни IMMEDIATE недоступны, используйте режим FIFO (первым пришел — первым ушел).
	}

	// Переход layout изображения swapchain для цикла рендеринга / презентации:
	// UNDEFINED → PRESENT_SRC_KHR(инициализация swapchain)
	// PRESENT_SRC_KHR ↔ GENERAL(цикл рендеринга)
	void Swapchain_VK::CmdTransitionSwapchainLayout(VkCommandBuffer cmd, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout)
	{
		VkPipelineStageFlags2 srcStage{ 0 };
		VkPipelineStageFlags2 dstStage{ 0 };
		VkAccessFlags2 srcAccess{ 0 };
		VkAccessFlags2 dstAccess{ 0 };

		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
		{
			// Swapchain initialization
			srcStage = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
			srcAccess = VK_ACCESS_2_NONE;
			dstStage = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
			dstAccess = VK_ACCESS_2_NONE;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR && newLayout == VK_IMAGE_LAYOUT_GENERAL)
		{
			// Before rendering
			srcStage = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
			srcAccess = VK_ACCESS_2_NONE;
			dstStage = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_2_TRANSFER_BIT;
			dstAccess = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_GENERAL && newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
		{
			// After rendering
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
#endif // ZRENDER_API_VULKAN
