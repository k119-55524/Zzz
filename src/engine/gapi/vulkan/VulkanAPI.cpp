#include <cstring>

#include "engine/gapi/vulkan/VulkanAPI.h"
#include "engine/gapi/GAPIDebugLogger.h"

#if defined(Z_VULKAN)

namespace zzz::engine
{
	VulkanAPI::~VulkanAPI()
	{
		if (m_Device)
		{
			vkDeviceWaitIdle(m_Device);

			if (m_RenderFence)
				vkDestroyFence(m_Device, m_RenderFence, nullptr);

			vkDestroyDevice(m_Device, nullptr);
			m_Device = VK_NULL_HANDLE;
		}

		if (m_Instance)
		{
			if (m_DebugMessenger)
			{
				auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_Instance, "vkDestroyDebugUtilsMessengerEXT");
				if (func)
					func(m_Instance, m_DebugMessenger, nullptr);
			}

			vkDestroyInstance(m_Instance, nullptr);
			m_Instance = VK_NULL_HANDLE;
		}
	}

#pragma region Initialize
	void VulkanAPI::Initialize(std::shared_ptr<UserSettingsManager> userSettings)
	{
		CreateInstance();
		EnableDebugMessenger();
		SelectPhysicalDeviceAndCreateLogicalDevice(userSettings);

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		VkResult vr = vkCreateFence(m_Device, &fenceInfo, nullptr, &m_RenderFence);
		if (vr != VK_SUCCESS)
			THROW_RUNTIME("[VulkanAPI::Initialize] Failed to create Render Fence: 0x{:08X}", static_cast<uint32_t>(vr));

		m_IsCanDisableVSync = true;

		DOut("[VulkanAPI::Initialize] Successfully initialized Vulkan 1.4 API.");
	}

	void VulkanAPI::CreateInstance()
	{
		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Zzz Engine App";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "Zzz Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = c_DefaultVulkanApiVersion; // VK_API_VERSION_1_4 из Constants.h

		std::vector<const char*> instanceExtensions = BuildInstanceExtensions();
		std::vector<const char*> instanceLayers = FindValidationLayers();

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;
		createInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size());
		createInfo.ppEnabledExtensionNames = instanceExtensions.data();
		createInfo.enabledLayerCount = static_cast<uint32_t>(instanceLayers.size());
		createInfo.ppEnabledLayerNames = instanceLayers.data();

		std::vector<VkLayerSettingEXT> layerSettings;
		VkLayerSettingsCreateInfoEXT layerSettingsCreateInfo{};
		if (BuildVerboseValidationLayerSettings(!instanceLayers.empty(), layerSettings, layerSettingsCreateInfo))
			createInfo.pNext = &layerSettingsCreateInfo;

		VkResult vr = vkCreateInstance(&createInfo, nullptr, &m_Instance);
		if (vr != VK_SUCCESS)
			THROW_RUNTIME("[VulkanAPI::CreateInstance] Failed to create VkInstance: 0x{:08X}", static_cast<uint32_t>(vr));
	}

	std::vector<const char*> VulkanAPI::BuildInstanceExtensions() const
	{
		std::vector<const char*> extensions = {
			VK_KHR_SURFACE_EXTENSION_NAME
		};

#if defined(Z_WINDOWS)
		extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(Z_ANDROID)
		extensions.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#elif defined(Z_LINUX)
#if defined(USE_WAYLAND)
		extensions.push_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
#else
		extensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#endif
#endif

#if Z_DEBUG_BUILD || Z_DEVELOPMENT_BUILD
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

		return extensions;
	}

	std::vector<const char*> VulkanAPI::FindValidationLayers() const
	{
#if Z_DEBUG_BUILD || Z_DEVELOPMENT_BUILD
		uint32_t layerCount = 0;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (const auto& layer : availableLayers)
		{
			if (std::strcmp(layer.layerName, c_ValidationLayerName) == 0)
				return { c_ValidationLayerName };
		}

		DOutWarning("[VulkanAPI::FindValidationLayers] VK_LAYER_KHRONOS_validation not found - Vulkan Validation Layers disabled.");
#endif // Z_DEBUG_BUILD || Z_DEVELOPMENT_BUILD

		return {};
	}

	bool VulkanAPI::BuildVerboseValidationLayerSettings(bool validationLayerFound, std::vector<VkLayerSettingEXT>& outLayerSettings, VkLayerSettingsCreateInfoEXT& outLayerSettingsCreateInfo) const
	{
		// Расширенные настройки VK_LAYER_KHRONOS_validation через VK_EXT_layer_settings - включаются только
		// в Debug/Development сборках под Z_GAPI_VERBOSE_DEBUG_LAYER, т.к. заметно увеличивают объём проверок
		// (и, соответственно, лога). Без этого дефайна слой работает с настройками по умолчанию, как и раньше.
#if (Z_DEBUG_BUILD || Z_DEVELOPMENT_BUILD) && defined(Z_GAPI_VERBOSE_DEBUG_LAYER)
		if (!validationLayerFound)
			return false;

		static VkBool32 s_LayerSettingTrue = VK_TRUE;

		outLayerSettings =
		{
			{ c_ValidationLayerName, "fine_grained_locking",  VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &s_LayerSettingTrue },
			{ c_ValidationLayerName, "validate_core",         VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &s_LayerSettingTrue },
			{ c_ValidationLayerName, "check_image_layout",    VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &s_LayerSettingTrue },
			{ c_ValidationLayerName, "check_command_buffer",  VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &s_LayerSettingTrue },
			{ c_ValidationLayerName, "check_object_in_use",   VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &s_LayerSettingTrue },
			{ c_ValidationLayerName, "check_query",           VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &s_LayerSettingTrue },
			{ c_ValidationLayerName, "check_shaders",         VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &s_LayerSettingTrue },
			{ c_ValidationLayerName, "check_shaders_caching", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &s_LayerSettingTrue },
			{ c_ValidationLayerName, "unique_handles",        VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &s_LayerSettingTrue },
			{ c_ValidationLayerName, "object_lifetime",       VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &s_LayerSettingTrue },
			{ c_ValidationLayerName, "stateless_param",       VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &s_LayerSettingTrue },
			{ c_ValidationLayerName, "debug_action",          VK_LAYER_SETTING_TYPE_STRING_EXT, uint32_t(c_ValidationDebugAction.size()), c_ValidationDebugAction.data() },
			{ c_ValidationLayerName, "report_flags",          VK_LAYER_SETTING_TYPE_STRING_EXT, uint32_t(c_GAPIDebugReportFlags.size()), c_GAPIDebugReportFlags.data() },
		};

		outLayerSettingsCreateInfo.sType = VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT;
		outLayerSettingsCreateInfo.settingCount = static_cast<uint32_t>(outLayerSettings.size());
		outLayerSettingsCreateInfo.pSettings = outLayerSettings.data();
		return true;
#else // Z_DEBUG_BUILD || Z_DEVELOPMENT_BUILD
		(void)validationLayerFound;
		(void)outLayerSettings;
		(void)outLayerSettingsCreateInfo;
		return false;
#endif // (Z_DEBUG_BUILD || Z_DEVELOPMENT_BUILD) && defined(Z_GAPI_VERBOSE_DEBUG_LAYER)
	}

	void VulkanAPI::EnableDebugMessenger()
	{
#if Z_DEBUG_BUILD || Z_DEVELOPMENT_BUILD
		VkDebugUtilsMessengerCreateInfoEXT debugInfo{};
		debugInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		debugInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
#if defined(Z_GAPI_VERBOSE_DEBUG_LAYER)
		debugInfo.messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
#endif // Z_GAPI_VERBOSE_DEBUG_LAYER
		debugInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		debugInfo.pfnUserCallback = [](VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type, const VkDebugUtilsMessengerCallbackDataEXT* data, void*) -> VkBool32
		{
			if (!data || !data->pMessage)
				return VK_FALSE;

			eGAPIDebugSeverity gapiSeverity = eGAPIDebugSeverity::Verbose;
			if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
				gapiSeverity = eGAPIDebugSeverity::Error;
			else if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
				gapiSeverity = eGAPIDebugSeverity::Warning;
			else if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
				gapiSeverity = eGAPIDebugSeverity::Info;

			eGAPIDebugCategory gapiCategory = eGAPIDebugCategory::General;
			if (type & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
				gapiCategory = eGAPIDebugCategory::Validation;
			else if (type & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT)
				gapiCategory = eGAPIDebugCategory::Performance;

			std::string_view message(data->pMessage);
			size_t start = 0;
			while (start < message.length())
			{
				size_t end = message.find('\n', start);
				if (end == std::string_view::npos)
					end = message.length();

				std::string_view line = message.substr(start, end - start);
				size_t first = line.find_first_not_of(" \t\r\n");
				if (first != std::string_view::npos)
				{
					size_t last = line.find_last_not_of(" \t\r\n");
					std::string_view trimmedLine = line.substr(first, (last - first + 1));

					// Игнорируем специфичные текстовые разделители '||' и заголовки стека слоев
					if (trimmedLine.find("||") == std::string_view::npos &&
					    trimmedLine.find("callstack setup") == std::string_view::npos)
					{
						std::string indented(trimmedLine);
						if (gapiSeverity == eGAPIDebugSeverity::Info)
						{
							static bool s_HeaderPrinted = false;
							if (!s_HeaderPrinted)
							{
								GAPIDebugLogger::Report(eGAPIType::Vulkan, eGAPIDebugSeverity::Info, eGAPIDebugCategory::General,
									"========== Vulkan Validation Layers ==========");
								s_HeaderPrinted = true;
							}

							std::string_view indent = "  ";
							if (trimmedLine.starts_with("VK_LAYER_") || trimmedLine.starts_with("Using "))
							{
								indent = "    ";
							}
							else if (trimmedLine.starts_with("Type:") || trimmedLine.starts_with("Enabled By:") ||
									 trimmedLine.starts_with("Disable Env Var:") || trimmedLine.starts_with("Manifest:") ||
									 trimmedLine.starts_with("Library:"))
							{
								indent = "      ";
							}

							indented = std::format("{}{}", indent, trimmedLine);
						}

						GAPIDebugLogger::Report(eGAPIType::Vulkan, gapiSeverity, gapiCategory, indented);
					}
				}

				start = end + 1;
			}
			return VK_FALSE;
		};

		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_Instance, "vkCreateDebugUtilsMessengerEXT");
		if (func)
			func(m_Instance, &debugInfo, nullptr, &m_DebugMessenger);
#endif // Z_DEBUG_BUILD || Z_DEVELOPMENT_BUILD
	}

	void VulkanAPI::SelectPhysicalDeviceAndCreateLogicalDevice(std::shared_ptr<UserSettingsManager> userSettings)
	{
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr);
		if (deviceCount == 0)
			THROW_RUNTIME("[VulkanAPI] No physical GPUs with Vulkan support found.");

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(m_Instance, &deviceCount, devices.data());

		VulkanGpuSelector selector(userSettings);
		for (auto device : devices)
		{
			selector.AddCandidate(device, VK_NULL_HANDLE);
		}

		auto bestCandidate = selector.SelectBestGpu();
		m_PhysicalDevice = bestCandidate.device;
		m_GraphicsQueueFamilyIndex = bestCandidate.graphicsQueueFamilyIndex;
		m_PresentQueueFamilyIndex = bestCandidate.presentQueueFamilyIndex;

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> uniqueQueueFamilies = { m_GraphicsQueueFamilyIndex, m_PresentQueueFamilyIndex };

		float queuePriority = 1.0f;
		for (uint32_t queueFamily : uniqueQueueFamilies)
		{
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		std::vector<const char*> deviceExtensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};

		// Vulkan 1.3/1.4 Dynamic Rendering Features
		VkPhysicalDeviceVulkan13Features vulkan13Features{};
		vulkan13Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
		vulkan13Features.dynamicRendering = VK_TRUE;
		vulkan13Features.synchronization2 = VK_TRUE;

		VkPhysicalDeviceFeatures2 deviceFeatures2{};
		deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		deviceFeatures2.pNext = &vulkan13Features;

		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pNext = &deviceFeatures2;
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();

		VkResult vr = vkCreateDevice(m_PhysicalDevice, &createInfo, nullptr, &m_Device);
		if (vr != VK_SUCCESS)
			THROW_RUNTIME("[VulkanAPI] Failed to create logical VkDevice: 0x{:08X}", static_cast<uint32_t>(vr));

		vkGetDeviceQueue(m_Device, m_GraphicsQueueFamilyIndex, 0, &m_GraphicsQueue);
		vkGetDeviceQueue(m_Device, m_PresentQueueFamilyIndex, 0, &m_PresentQueue);
	}
#pragma endregion

	void VulkanAPI::WaitForGpu()
	{
		if (m_Device)
			vkDeviceWaitIdle(m_Device);
	}
}
#endif // Z_VULKAN
