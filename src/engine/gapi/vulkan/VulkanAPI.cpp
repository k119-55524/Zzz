#include "engine/gapi/vulkan/VulkanAPI.h"

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

		std::vector<const char*> instanceExtensions = {
			VK_KHR_SURFACE_EXTENSION_NAME
		};

#if defined(Z_WINDOWS)
		instanceExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(Z_ANDROID)
		instanceExtensions.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#elif defined(Z_LINUX)
#if defined(USE_WAYLAND)
		instanceExtensions.push_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
#else
		instanceExtensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#endif
#endif

		std::vector<const char*> instanceLayers;
#if defined(Z_DEBUG_BUILD)
		instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		instanceLayers.push_back("VK_LAYER_KHRONOS_validation");
#endif

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;
		createInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size());
		createInfo.ppEnabledExtensionNames = instanceExtensions.data();
		createInfo.enabledLayerCount = static_cast<uint32_t>(instanceLayers.size());
		createInfo.ppEnabledLayerNames = instanceLayers.data();

		VkResult vr = vkCreateInstance(&createInfo, nullptr, &m_Instance);
		if (vr != VK_SUCCESS)
			THROW_RUNTIME("[VulkanAPI::CreateInstance] Failed to create VkInstance: 0x{:08X}", static_cast<uint32_t>(vr));
	}

	void VulkanAPI::EnableDebugMessenger()
	{
#if defined(Z_DEBUG_BUILD)
		VkDebugUtilsMessengerCreateInfoEXT debugInfo{};
		debugInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		debugInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		debugInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		debugInfo.pfnUserCallback = [](VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT, const VkDebugUtilsMessengerCallbackDataEXT* data, void*) -> VkBool32
		{
			if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
				DOutError("[Vulkan Validation Error] {}", data->pMessage);
			else if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
				DOut("[Vulkan Validation Warning] {}", data->pMessage);
			else
				DOut("[Vulkan Validation Info] {}", data->pMessage);
			return VK_FALSE;
		};

		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_Instance, "vkCreateDebugUtilsMessengerEXT");
		if (func)
			func(m_Instance, &debugInfo, nullptr, &m_DebugMessenger);
#endif
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

	void VulkanAPI::SubmitCommandLists()
	{
	}

	void VulkanAPI::BeginRender()
	{
	}

	void VulkanAPI::EndRender()
	{
	}

	void VulkanAPI::WaitForGpu()
	{
		if (m_Device)
			vkDeviceWaitIdle(m_Device);
	}
}

#endif // Z_VULKAN
