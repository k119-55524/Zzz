#pragma once

#include "engine/gapi/IGAPI.h"
#include "engine/gapi/selectors/gpu/vulkan/VulkanGpuSelector.h"

#include <mutex>

#if defined(Z_VULKAN)

namespace zzz::engine
{
	// Сопоставление Vk*-хендлов их VkObjectType - нужно для vkSetDebugUtilsObjectNameEXT в VulkanAPI::SetDebugName.
	// Перегрузки заведены только для типов, реально создаваемых в этом проекте (Swapchain_VK/DepthBuffer_VK/SurfView_VK).
	constexpr VkObjectType GetVulkanObjectType(VkSurfaceKHR) noexcept { return VK_OBJECT_TYPE_SURFACE_KHR; }
	constexpr VkObjectType GetVulkanObjectType(VkSwapchainKHR) noexcept { return VK_OBJECT_TYPE_SWAPCHAIN_KHR; }
	constexpr VkObjectType GetVulkanObjectType(VkImage) noexcept { return VK_OBJECT_TYPE_IMAGE; }
	constexpr VkObjectType GetVulkanObjectType(VkImageView) noexcept { return VK_OBJECT_TYPE_IMAGE_VIEW; }
	constexpr VkObjectType GetVulkanObjectType(VkDeviceMemory) noexcept { return VK_OBJECT_TYPE_DEVICE_MEMORY; }
	constexpr VkObjectType GetVulkanObjectType(VkFence) noexcept { return VK_OBJECT_TYPE_FENCE; }
	constexpr VkObjectType GetVulkanObjectType(VkSemaphore) noexcept { return VK_OBJECT_TYPE_SEMAPHORE; }
	constexpr VkObjectType GetVulkanObjectType(VkCommandPool) noexcept { return VK_OBJECT_TYPE_COMMAND_POOL; }
	constexpr VkObjectType GetVulkanObjectType(VkCommandBuffer) noexcept { return VK_OBJECT_TYPE_COMMAND_BUFFER; }

	class VulkanAPI final : public IGAPI
	{
	public:
		explicit VulkanAPI() = default;
		~VulkanAPI() override;

		void WaitForGpu() override;
		void Initialize(std::shared_ptr<UserSettingsManager> userSettings) override;

		[[nodiscard]] VkInstance GetInstance() const noexcept { return m_Instance; }
		[[nodiscard]] VkPhysicalDevice GetPhysicalDevice() const noexcept { return m_PhysicalDevice; }
		[[nodiscard]] VkDevice GetDevice() const noexcept { return m_Device; }
		[[nodiscard]] VkQueue GetGraphicsQueue() const noexcept { return m_GraphicsQueue; }
		[[nodiscard]] VkQueue GetPresentQueue() const noexcept { return m_PresentQueue; }
		[[nodiscard]] uint32_t GetGraphicsQueueFamilyIndex() const noexcept { return m_GraphicsQueueFamilyIndex; }
		[[nodiscard]] uint32_t GetPresentQueueFamilyIndex() const noexcept { return m_PresentQueueFamilyIndex; }

		// Потокобезопасные обёртки над vkQueueSubmit/vkQueuePresentKHR - VkQueue общий на все View (GAPI один
		// shared_ptr на всё приложение), а спецификация Vulkan требует внешней синхронизации доступа к очереди
		// при конкурентных вызовах с разных потоков. Использовать ТОЛЬКО эти методы, не GetGraphicsQueue()/
		// GetPresentQueue() напрямую для submit/present.
		VkResult QueueSubmit(uint32_t submitCount, const VkSubmitInfo* submits, VkFence fence);
		VkResult QueuePresent(const VkPresentInfoKHR* presentInfo);

		template<typename T = void>
		void RegisterPendingTransition([[maybe_unused]] T* resource = nullptr) noexcept {}

		template<typename T = void>
		void FlushPendingTransitions([[maybe_unused]] T* cmdList = nullptr) noexcept {}

		template<typename T>
		void SetDebugName(T handle, const char* name) const
		{
#if Z_DEBUG_BUILD || Z_DEVELOPMENT_BUILD
			if (!m_Device || !handle || !name)
				return;

			auto func = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetDeviceProcAddr(m_Device, "vkSetDebugUtilsObjectNameEXT");
			if (!func)
				return;

			VkDebugUtilsObjectNameInfoEXT info{};
			info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
			info.objectType = GetVulkanObjectType(handle);
			info.objectHandle = reinterpret_cast<uint64_t>(handle);
			info.pObjectName = name;

			func(m_Device, &info);
#endif
		}

	private:
		// Имя слоя и его debug_action для VK_EXT_layer_settings - используются и в FindValidationLayers
		// (поиск/сравнение), и в BuildVerboseValidationLayerSettings (заполнение VkLayerSettingEXT), поэтому
		// вынесены в константы класса, а не дублируются как локальные литералы в каждой функции.
		static constexpr const char* c_ValidationLayerName = "VK_LAYER_KHRONOS_validation";
		static constexpr std::array<const char*, 1> c_ValidationDebugAction = { "VK_DBG_LAYER_ACTION_LOG_MSG" };
		static constexpr std::array<const char*, 4> c_GAPIDebugReportFlags = { "info", "warn", "perf", "error" };

		void CreateInstance();

		// Базовый набор расширений инстанса (surface-расширение под текущую платформу + VK_EXT_debug_utils
		// в Debug/Development сборках). Просто список строк - в отличие от VkInstanceCreateInfo, ничего не
		// хранит по указателю на другие объекты, поэтому безопасно возвращается по значению.
		std::vector<const char*> BuildInstanceExtensions() const;

		// Ищет VK_LAYER_KHRONOS_validation среди доступных слоёв (только в Debug/Development сборках) и,
		// если найден, возвращает его имя единственным элементом списка (иначе - пустой список). Список
		// уже сам по себе говорит, найден ли слой - отдельный bool не нужен, проверяйте !empty().
		std::vector<const char*> FindValidationLayers() const;

		// Заполняет расширенные настройки VK_LAYER_KHRONOS_validation (VK_EXT_layer_settings) - активна
		// только в Debug/Development сборках при включённом Z_GAPI_VERBOSE_DEBUG_LAYER (заметно увеличивают
		// объём проверок и, соответственно, лога), вне этих условий не делает ничего и возвращает false -
		// вызывающему коду не нужно оборачивать вызов в свой #if.
		// outLayerSettings/outLayerSettingsCreateInfo передаются по ссылке и должны жить у вызывающего кода
		// до вызова vkCreateInstance - VkLayerSettingsCreateInfoEXT хранит указатель на данные вектора.
		bool BuildVerboseValidationLayerSettings(bool validationLayerFound, std::vector<VkLayerSettingEXT>& outLayerSettings, VkLayerSettingsCreateInfoEXT& outLayerSettingsCreateInfo) const;

		void EnableDebugMessenger();

		void SelectPhysicalDeviceAndCreateLogicalDevice(std::shared_ptr<UserSettingsManager> userSettings);

		VkInstance m_Instance{ VK_NULL_HANDLE };
		VkDebugUtilsMessengerEXT m_DebugMessenger{ VK_NULL_HANDLE };
		VkPhysicalDevice m_PhysicalDevice{ VK_NULL_HANDLE };
		VkDevice m_Device{ VK_NULL_HANDLE };
		VkQueue m_GraphicsQueue{ VK_NULL_HANDLE };
		VkQueue m_PresentQueue{ VK_NULL_HANDLE };

		uint32_t m_GraphicsQueueFamilyIndex{ UINT32_MAX };
		uint32_t m_PresentQueueFamilyIndex{ UINT32_MAX };

		std::mutex m_QueueMutex;
	};
}

#endif // Z_VULKAN
