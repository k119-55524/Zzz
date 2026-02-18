
export module VKAPI;

#if defined(ZRENDER_API_VULKAN)

import IGAPI;
import Result;
import StrConvert;
import GAPIConfig;
import GPUUploadVK;
import EngineConstants;
import VKDeviceCapabilities;

using namespace zzz;
using namespace zzz::core;

namespace zzz::vk
{
	// Структура-кандидат для хранения лучшего устройства
	struct Candidate
	{
		VkPhysicalDevice device{};
		uint32_t graphicsQueueFamily{};
		uint32_t computeQueueFamily{};
		uint32_t transferQueueFamily{};
		uint64_t score = 0;
	};

	class TestSurface_MSWin final
	{
		VkInstance m_Instance;
		VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
		HWND m_Hwnd = nullptr;

	public:
		explicit TestSurface_MSWin(VkInstance instance) :
			m_Instance(instance)
		{
			m_Hwnd = CreateWindowEx(
				0, L"STATIC", L"",
				WS_POPUP, 0, 0, 1, 1,
				nullptr, nullptr, GetModuleHandle(nullptr), nullptr);

			if (m_Hwnd)
			{
				VkWin32SurfaceCreateInfoKHR createInfo{
					.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
					.hinstance = GetModuleHandle(nullptr),
					.hwnd = m_Hwnd
				};
				vkCreateWin32SurfaceKHR(m_Instance, &createInfo, nullptr, &m_Surface);
			}
		}

		~TestSurface_MSWin()
		{
			if (m_Surface != VK_NULL_HANDLE)
				vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);

			if (m_Hwnd)
				DestroyWindow(m_Hwnd);
		}

		VkSurfaceKHR Get() const { return m_Surface; }
		bool IsValid() const { return m_Surface != VK_NULL_HANDLE; }
	};
}

namespace zzz::vk
{
	export class VKAPI final : public IGAPI
	{
		Z_NO_CREATE_COPY(VKAPI);

	public:
		explicit VKAPI(const std::shared_ptr<GAPIConfig> config);
		virtual ~VKAPI() override;

		inline VkInstance GetInstance() const noexcept { return m_Instance; }
		inline VkPhysicalDevice GetPhysicalDevice() const noexcept { return m_PhysicalDevice; }
		inline VkDevice GetDevice() const noexcept { return m_Device; }

		inline VkQueue GetGraphicsQueue() const noexcept { return m_GraphicsQueue; }
		inline VkQueue GetComputeQueue() const noexcept { return m_ComputeQueue; }
		inline VkQueue GetTransferQueue() const noexcept { return m_TransferQueue; }
		inline VkCommandPool GetGraphicsCommandPool() const noexcept { return m_GraphicsCommandPool; }
		inline VkCommandPool GetComputeCommandPool() const noexcept { return m_ComputeCommandPool; }
		inline VkCommandPool GetTransferCommandPool() const noexcept { return m_TransferCommandPool; }
		inline void SetFrameSyncData(VkSemaphore waitSemaphore, VkSemaphore signalSemaphore) noexcept
		{
			m_CurrentWaitSemaphore = waitSemaphore;
			m_CurrentSignalSemaphore = signalSemaphore;
		}
		inline VkCommandBuffer GetGraphicsCBUpdate() const noexcept { return m_GraphicsCommandBuffers[m_IndexFrameRender]; }

		void SubmitCommandLists() override;
		void BeginRender() override;

	protected:
		[[nodiscard]] Result<> Init() override;
		void WaitForGpu() override;
		void EndRender() override;

	private:
		[[nodiscard]] Result<> CreateInstance();
		[[nodiscard]] Result<std::vector<const char*>> GetRequiredExtensions();
		[[nodiscard]] const char* GetPlatformExtension();
		[[nodiscard]] Result<> CreateDebugMessenger();
		[[nodiscard]] Result<> PickPhysicalDevice(Candidate& deviceCandidate, const VkSurfaceKHR& surface);
		[[nodiscard]] std::optional<Candidate> BestDeviceCandidat(const std::vector<VkPhysicalDevice>& devices, const VkSurfaceKHR& surface);
		[[nodiscard]] Result<> CreateLogicalDevice(const Candidate& deviceCandidate);
		[[nodiscard]] Result<> CreateCommandPools(const Candidate& deviceCandidate);
		[[nodiscard]] Result<> CreateCommandBuffers();
		[[nodiscard]] Result<> CreateSyncObjects();

		VkInstance m_Instance;
		VkPhysicalDevice m_PhysicalDevice;
		VkDevice m_Device;

		VkQueue m_GraphicsQueue;
		VkQueue m_ComputeQueue;
		VkQueue m_TransferQueue;

		VkCommandPool m_GraphicsCommandPool;
		VkCommandPool m_TransferCommandPool;
		VkCommandPool m_ComputeCommandPool;
		std::vector<VkCommandBuffer> m_GraphicsCommandBuffers;
		std::vector<VkCommandBuffer> m_ComputeCommandBuffers;
		std::vector<VkCommandBuffer> m_TransferCommandBuffers;

		std::array<VkFence, BACK_BUFFER_COUNT> m_InFlightFences{};
		VkSemaphore m_CurrentWaitSemaphore = VK_NULL_HANDLE;
		VkSemaphore m_CurrentSignalSemaphore = VK_NULL_HANDLE;

#if defined(_DEBUG)
		VkDebugUtilsMessengerEXT m_DebugMessenger = VK_NULL_HANDLE;

		static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData);
		void ReportGPUDebugMessages(
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const char* message);
#endif
	};

	VKAPI::VKAPI(const std::shared_ptr<GAPIConfig> config) :
		IGAPI(config, eGAPIType::Vulkan),
		m_Instance(VK_NULL_HANDLE),
		m_PhysicalDevice(VK_NULL_HANDLE),
		m_Device(VK_NULL_HANDLE),
		m_GraphicsQueue(VK_NULL_HANDLE),
		m_ComputeQueue(VK_NULL_HANDLE),
		m_TransferQueue(VK_NULL_HANDLE),
		m_GraphicsCommandPool(VK_NULL_HANDLE),
		m_TransferCommandPool(VK_NULL_HANDLE),
		m_ComputeCommandPool(VK_NULL_HANDLE)
	{
		ensure(config, "GAPIConfig cannot be null.");

		for (uint32_t i = 0; i < BACK_BUFFER_COUNT; i++)
			m_InFlightFences[i] = VK_NULL_HANDLE;
	}

	VKAPI::~VKAPI()
	{
		if (m_Device)
			vkDeviceWaitIdle(m_Device);

		for (size_t i = 0; i < BACK_BUFFER_COUNT; i++)
		{
			if (m_InFlightFences[i])
				vkDestroyFence(m_Device, m_InFlightFences[i], nullptr);
		}

		if (m_ComputeCommandPool && m_ComputeCommandPool != m_GraphicsCommandPool)
			vkDestroyCommandPool(m_Device, m_ComputeCommandPool, nullptr);

		if (m_TransferCommandPool && m_TransferCommandPool != m_GraphicsCommandPool &&
			m_TransferCommandPool != m_ComputeCommandPool)
			vkDestroyCommandPool(m_Device, m_TransferCommandPool, nullptr);

		if (m_GraphicsCommandPool)
			vkDestroyCommandPool(m_Device, m_GraphicsCommandPool, nullptr);

		if (m_Device)
			vkDestroyDevice(m_Device, nullptr);

#if defined(_DEBUG)
		auto destroyFunc = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
			vkGetInstanceProcAddr(m_Instance, "vkDestroyDebugUtilsMessengerEXT"));
		if (destroyFunc && m_DebugMessenger)
			destroyFunc(m_Instance, m_DebugMessenger, nullptr);
#endif

		if (m_Instance)
			vkDestroyInstance(m_Instance, nullptr);
	}

#pragma region Initialize
	[[nodiscard]] Result<> VKAPI::Init()
	{
		VkResult vr = volkInitialize();
		if (vr != VK_SUCCESS)
			return Unexpected(eResult::failure, std::format(L"volkInitialize failed ({})", int(vr)));

		Candidate deviceCandidate;
		Result<> res = CreateInstance()
			.and_then([&]() { return CreateDebugMessenger(); })
			.and_then([&]()
				{
					TestSurface_MSWin surface(m_Instance);

					if (!surface.IsValid())
						return Result<>(Unexpected(eResult::failure, L"Failed to create TestSurface_MSWin surface"));

					return PickPhysicalDevice(deviceCandidate, surface.Get());
				})
			.and_then([&]() { return CreateLogicalDevice(deviceCandidate); })
			.and_then([&]() { return CreateCommandPools(deviceCandidate); })
			.and_then([&]() { return CreateCommandBuffers(); })
			.and_then([&]() { return CreateSyncObjects(); })
			.and_then([&]()
				{
					m_CheckGapiSupport = safe_make_unique<VKDeviceCapabilities>(m_PhysicalDevice, m_Device);
					m_CPUtoGPUDataTransfer = safe_make_unique<GPUUploadVK>();
				});

		return res;
	}

	[[nodiscard]] Result<> VKAPI::CreateInstance()
	{
		std::string appNameStr = wstring_to_string(m_Config->GetAppName());
		std::string engineNameStr = wstring_to_string(std::wstring(g_EngineName));
		const char* appName = appNameStr.c_str();
		const char* engineName = engineNameStr.c_str();
		const Version& appVersion = m_Config->GetAppVersion();

		uint32_t supportedVersion = VK_API_VERSION_1_0;
		if (vkEnumerateInstanceVersion)
			vkEnumerateInstanceVersion(&supportedVersion);

		uint32_t apiVersion = std::min(supportedVersion, VULKAN_ENGINE_MAX_VERSION);

		DebugOutput(std::format(
			L"Vulkan supported: {}.{}.{} | Using: {}.{}.{}",
			VK_VERSION_MAJOR(supportedVersion), VK_VERSION_MINOR(supportedVersion), VK_VERSION_PATCH(supportedVersion),
			VK_VERSION_MAJOR(apiVersion), VK_VERSION_MINOR(apiVersion), VK_VERSION_PATCH(apiVersion)));

		auto extensionsRes = GetRequiredExtensions();
		if (!extensionsRes)
			return extensionsRes.error();

		std::vector<const char*> extensions = extensionsRes.value();
		std::vector<const char*> layers;

#ifdef _DEBUG
		uint32_t layerCount = 0;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		bool validationLayerFound = false;
		for (const auto& layer : availableLayers)
		{
			if (std::strcmp(layer.layerName, "VK_LAYER_KHRONOS_validation") == 0)
			{
				validationLayerFound = true;
				break;
			}
		}

		if (validationLayerFound)
			layers.push_back("VK_LAYER_KHRONOS_validation");
		else
			DebugOutput(L"Warning: VK_LAYER_KHRONOS_validation not found");
#endif

		VkApplicationInfo appInfo
		{
			.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
			.pApplicationName = appName,
			.applicationVersion = VK_MAKE_VERSION(appVersion.GetMajor(), appVersion.GetMinor(), appVersion.GetPatch()),
			.pEngineName = engineName,
			.engineVersion = VK_MAKE_VERSION(g_EngineVersion.GetMajor(), g_EngineVersion.GetMinor(), g_EngineVersion.GetPatch()),
			.apiVersion = apiVersion
		};

		VkInstanceCreateInfo ci
		{
			.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.pApplicationInfo = &appInfo,
			.enabledLayerCount = uint32_t(layers.size()),
			.ppEnabledLayerNames = layers.empty() ? nullptr : layers.data(),
			.enabledExtensionCount = uint32_t(extensions.size()),
			.ppEnabledExtensionNames = extensions.data()
		};

		VkResult vr = vkCreateInstance(&ci, nullptr, &m_Instance);
		if (vr != VK_SUCCESS)
			return Unexpected(eResult::failure, std::format(L"vkCreateInstance failed ({})", int(vr)));

		volkLoadInstance(m_Instance);

		return {};
	}

	[[nodiscard]] Result<std::vector<const char*>> VKAPI::GetRequiredExtensions()
	{
		uint32_t extCount = 0;
		auto vkRes = vkEnumerateInstanceExtensionProperties(nullptr, &extCount, nullptr);
		if (vkRes != VK_SUCCESS)
			return Unexpected(eResult::failure, std::format(L"vkEnumerateInstanceExtensionProperties failed ({})", int(vkRes)));

		std::vector<VkExtensionProperties> availableExt(extCount);
		vkRes = vkEnumerateInstanceExtensionProperties(nullptr, &extCount, availableExt.data());
		if (vkRes != VK_SUCCESS)
			return Unexpected(eResult::failure, std::format(L"vkEnumerateInstanceExtensionProperties failed ({})", int(vkRes)));

		auto IsExtensionAvailable = [&](const char* name)
			{
				for (const auto& ext : availableExt)
				{
					if (std::strcmp(ext.extensionName, name) == 0)
						return true;
				}
				return false;
			};

		std::vector<const char*> extensions;

		auto RequireExtension = [&](const char* name) -> Result<>
			{
				if (!IsExtensionAvailable(name))
					return Unexpected(eResult::failure, std::format(L"Required extension not supported: {}", string_to_wstring(name).value_or(L"Unknown extension name")));

				extensions.push_back(name);
				return {};
			};

		std::vector<const char*> requiredExtensions =
		{
			VK_KHR_SURFACE_EXTENSION_NAME
		};

		requiredExtensions.push_back(GetPlatformExtension());

#ifdef _DEBUG
		if (IsExtensionAvailable(VK_EXT_DEBUG_UTILS_EXTENSION_NAME))
			requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

		for (const char* ext : requiredExtensions)
		{
			if (auto res = RequireExtension(ext); !res)
				return res.error();
		}

		return extensions;
	}

	[[nodiscard]] const char* VKAPI::GetPlatformExtension()
	{
#if defined(ZPLATFORM_MSWINDOWS)
		return VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
#elif defined(ZPLATFORM_ANDROID)
		return VK_KHR_ANDROID_SURFACE_EXTENSION_NAME;
#elif defined(ZPLATFORM_LINUX)
#if defined(USE_WAYLAND)
		return VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME;
#else
		return VK_KHR_XCB_SURFACE_EXTENSION_NAME;
#endif
#endif
	}

#if defined(_DEBUG)
	VKAPI_ATTR VkBool32 VKAPI_CALL VKAPI::DebugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData)
	{
		auto* api = reinterpret_cast<VKAPI*>(pUserData);
		if (api)
			api->ReportGPUDebugMessages(messageSeverity, messageType, pCallbackData->pMessage);

		return VK_FALSE;
	}

	void VKAPI::ReportGPUDebugMessages(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const char* message)
	{
		std::wstring severityStr;
		if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
			severityStr = L"VERBOSE";
		else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
			severityStr = L"INFO";
		else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
			severityStr = L"WARNING";
		else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
			severityStr = L"ERROR";
		else
			severityStr = L"UNKNOWN";

		std::wstring typeStr;
		if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT)
			typeStr = L"GENERAL";
		else if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
			typeStr = L"VALIDATION";
		else if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT)
			typeStr = L"PERFORMANCE";
		else
			typeStr = L"UNKNOWN";

		LogGPUDebugMessage(std::format(L"[Vulkan Validation] {} | {}: {}", severityStr, typeStr, string_to_wstring(message).value_or(L"Unknown message")));
	}
#endif

	[[nodiscard]] Result<> VKAPI::CreateDebugMessenger()
	{
#ifdef _DEBUG
		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
		debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		debugCreateInfo.messageSeverity =
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		debugCreateInfo.messageType =
			VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		debugCreateInfo.pfnUserCallback = VKAPI::DebugCallback;
		debugCreateInfo.pUserData = this;

		auto func =
			reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(m_Instance, "vkCreateDebugUtilsMessengerEXT"));

		if (func && func(m_Instance, &debugCreateInfo, nullptr, &m_DebugMessenger) != VK_SUCCESS)
			return Unexpected(eResult::failure, L"Failed to create debug messenger");
#endif

		return {};
	}

	[[nodiscard]] Result<> VKAPI::PickPhysicalDevice(Candidate& deviceCandidate, const VkSurfaceKHR& surface)
	{
		uint32_t deviceCount = 0;
		auto vkRes = vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr);
		if (vkRes != VK_SUCCESS)
			return Unexpected(eResult::failure, std::format(L"vkEnumeratePhysicalDevices failed ({})", int(vkRes)));

		if (deviceCount == 0)
			return Unexpected(eResult::failure, L"No Vulkan devices found");

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkRes = vkEnumeratePhysicalDevices(m_Instance, &deviceCount, devices.data());
		if (vkRes != VK_SUCCESS)
			return Unexpected(eResult::failure, std::format(L"vkEnumeratePhysicalDevices failed ({})", int(vkRes)));

		auto candidat = BestDeviceCandidat(devices, surface);
		if (!candidat)
			return Unexpected(eResult::failure, L"No suitable Vulkan device found");

		deviceCandidate = candidat.value();
		m_PhysicalDevice = deviceCandidate.device;

		VkPhysicalDeviceProperties props{};
		vkGetPhysicalDeviceProperties(m_PhysicalDevice, &props);
		DebugOutput(std::format(L"Selected GPU: {} (score: {})", string_to_wstring(props.deviceName).value_or(L"Unknown GPU"), deviceCandidate.score));

		return {};
	}

	[[nodiscard]] std::optional<Candidate> VKAPI::BestDeviceCandidat(const std::vector<VkPhysicalDevice>& devices, const VkSurfaceKHR& surface)
	{
		std::optional<zzz::vk::Candidate> best;

		for (VkPhysicalDevice device : devices)
		{
			uint32_t familyCount = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(device, &familyCount, nullptr);

			std::vector<VkQueueFamilyProperties> families(familyCount);
			vkGetPhysicalDeviceQueueFamilyProperties(device, &familyCount, families.data());

			std::optional<uint32_t> graphicsFamily;
			std::optional<uint32_t> computeFamily;
			std::optional<uint32_t> transferFamily;

			for (uint32_t i = 0; i < familyCount; ++i)
			{
				const auto& f = families[i];

				if ((f.queueFlags & VK_QUEUE_GRAPHICS_BIT) && !graphicsFamily.has_value())
					graphicsFamily = i;

				if ((f.queueFlags & VK_QUEUE_COMPUTE_BIT) && !(f.queueFlags & VK_QUEUE_GRAPHICS_BIT) && !computeFamily.has_value())
					computeFamily = i;

				if ((f.queueFlags & VK_QUEUE_TRANSFER_BIT) && !(f.queueFlags & VK_QUEUE_GRAPHICS_BIT) && !(f.queueFlags & VK_QUEUE_COMPUTE_BIT) && !transferFamily.has_value())
					transferFamily = i;
			}

			if (!computeFamily) computeFamily = graphicsFamily;
			if (!transferFamily) transferFamily = graphicsFamily;

			if (!graphicsFamily)
				continue;

			VkPhysicalDeviceProperties props{};
			vkGetPhysicalDeviceProperties(device, &props);

			VkPhysicalDeviceMemoryProperties memProps{};
			vkGetPhysicalDeviceMemoryProperties(device, &memProps);

			uint64_t localMemory = 0;
			for (uint32_t i = 0; i < memProps.memoryHeapCount; ++i)
			{
				if (memProps.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
					localMemory += memProps.memoryHeaps[i].size;
			}

			VkPhysicalDeviceVulkan12Features features12{
				.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES
			};

			VkPhysicalDeviceVulkan13Features features13{
				.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
				.pNext = &features12
			};

			VkPhysicalDeviceFeatures2 features2{
				.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
				.pNext = &features13
			};

			vkGetPhysicalDeviceFeatures2(device, &features2);

			if (!features2.features.samplerAnisotropy ||
				!features2.features.multiDrawIndirect ||
				!features2.features.fragmentStoresAndAtomics ||
				!features2.features.independentBlend)
				continue;

			if (!features12.descriptorIndexing ||
				!features12.runtimeDescriptorArray ||
				!features12.bufferDeviceAddress ||
				!features12.timelineSemaphore)
				continue;

			if (!features13.dynamicRendering)
				continue;

			uint64_t score = 0;

			if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
				score += 1'000'000;

			score += localMemory / (1024ull * 1024ull);
			score += VK_VERSION_MAJOR(props.apiVersion) * 10'000;
			score += VK_VERSION_MINOR(props.apiVersion) * 1'000;
			score += families.size() * 100;

			if (!best || score > best->score)
			{
				best = Candidate{
					.device = device,
					.graphicsQueueFamily = *graphicsFamily,
					.computeQueueFamily = *computeFamily,
					.transferQueueFamily = *transferFamily,
					.score = score
				};
			}
		}

		return best;
	}

	[[nodiscard]] Result<> VKAPI::CreateLogicalDevice(const Candidate& deviceCandidate)
	{
		float priority = 1.0f;

		std::vector<VkDeviceQueueCreateInfo> queueInfos;
		std::set<uint32_t> usedFamilies;

		auto AddQueue = [&](uint32_t familyIndex)
			{
				if (usedFamilies.find(familyIndex) != usedFamilies.end())
					return;

				VkDeviceQueueCreateInfo qi{};
				qi.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				qi.queueFamilyIndex = familyIndex;
				qi.queueCount = 1;
				qi.pQueuePriorities = &priority;

				queueInfos.push_back(qi);
				usedFamilies.insert(familyIndex);
			};

		AddQueue(deviceCandidate.graphicsQueueFamily);
		AddQueue(deviceCandidate.computeQueueFamily);
		AddQueue(deviceCandidate.transferQueueFamily);

		uint32_t extCount = 0;
		auto vkRes = vkEnumerateDeviceExtensionProperties(m_PhysicalDevice, nullptr, &extCount, nullptr);
		if (vkRes != VK_SUCCESS)
			return Unexpected(eResult::failure, std::format(L"vkEnumerateDeviceExtensionProperties failed ({})", int(vkRes)));

		std::vector<VkExtensionProperties> availableExt(extCount);
		vkRes = vkEnumerateDeviceExtensionProperties(m_PhysicalDevice, nullptr, &extCount, availableExt.data());
		if (vkRes != VK_SUCCESS)
			return Unexpected(eResult::failure, std::format(L"vkEnumerateDeviceExtensionProperties failed ({})", int(vkRes)));

		auto IsExtensionAvailable = [&](const char* name)
			{
				for (const auto& ext : availableExt)
				{
					if (std::strcmp(ext.extensionName, name) == 0)
						return true;
				}
				return false;
			};

		std::vector<const char*> enabledExtensions;

		auto RequireExtension = [&](const char* name) -> Result<>
			{
				if (!IsExtensionAvailable(name))
					return Unexpected(eResult::failure, std::format(L"Required device extension not supported: {}", string_to_wstring(name).value_or(L"Unknown extension name")));

				enabledExtensions.push_back(name);
				return {};
			};

		auto TryAddExtension = [&](const char* name)
			{
				if (IsExtensionAvailable(name))
					enabledExtensions.push_back(name);
			};

		std::vector<const char*> requiredExtensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
			VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME,
			VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
			VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
			VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME
		};

		for (const char* ext : requiredExtensions)
		{
			if (auto res = RequireExtension(ext); !res)
				return res.error();
		}

		TryAddExtension(VK_EXT_MESH_SHADER_EXTENSION_NAME);

#if defined(_DEBUG)
		TryAddExtension(VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME);
#endif

		VkPhysicalDeviceFeatures2 features2{};
		features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;

		VkPhysicalDeviceVulkan12Features features12{};
		features12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
		features12.timelineSemaphore = VK_TRUE;
		features12.bufferDeviceAddress = VK_TRUE;
		features12.descriptorIndexing = VK_TRUE;
		features12.runtimeDescriptorArray = VK_TRUE;

		VkPhysicalDeviceVulkan13Features features13{};
		features13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
		features13.dynamicRendering = VK_TRUE;

		features2.pNext = &features12;
		features12.pNext = &features13;

		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pNext = &features2;
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueInfos.size());
		createInfo.pQueueCreateInfos = queueInfos.data();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(enabledExtensions.size());
		createInfo.ppEnabledExtensionNames = enabledExtensions.data();
		createInfo.pEnabledFeatures = nullptr;

		if (vkCreateDevice(deviceCandidate.device, &createInfo, nullptr, &m_Device) != VK_SUCCESS)
			return Unexpected(eResult::failure, L"Failed to create logical device");

		vkGetDeviceQueue(m_Device, deviceCandidate.graphicsQueueFamily, 0, &m_GraphicsQueue);
		vkGetDeviceQueue(m_Device, deviceCandidate.computeQueueFamily, 0, &m_ComputeQueue);
		vkGetDeviceQueue(m_Device, deviceCandidate.transferQueueFamily, 0, &m_TransferQueue);

		return {};
	}

	[[nodiscard]] Result<> VKAPI::CreateCommandPools(const Candidate& deviceCandidate)
	{
		auto CreatePool = [&](uint32_t queueFamily, VkCommandPool& pool) -> Result<>
			{
				VkCommandPoolCreateInfo ci{
					.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
					.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
					.queueFamilyIndex = queueFamily
				};

				VkResult vr = vkCreateCommandPool(m_Device, &ci, nullptr, &pool);
				if (vr != VK_SUCCESS)
					return Unexpected(eResult::failure, std::format(L"vkCreateCommandPool failed ({})", int(vr)));

				return {};
			};

		if (auto res = CreatePool(deviceCandidate.graphicsQueueFamily, m_GraphicsCommandPool); !res)
			return res;

		if (deviceCandidate.computeQueueFamily != deviceCandidate.graphicsQueueFamily)
		{
			if (auto res = CreatePool(deviceCandidate.computeQueueFamily, m_ComputeCommandPool); !res)
				return res;
		}
		else
		{
			m_ComputeCommandPool = m_GraphicsCommandPool;
		}

		if (deviceCandidate.transferQueueFamily != deviceCandidate.graphicsQueueFamily &&
			deviceCandidate.transferQueueFamily != deviceCandidate.computeQueueFamily)
		{
			if (auto res = CreatePool(deviceCandidate.transferQueueFamily, m_TransferCommandPool); !res)
				return res;
		}
		else if (deviceCandidate.transferQueueFamily == deviceCandidate.computeQueueFamily)
		{
			m_TransferCommandPool = m_ComputeCommandPool;
		}
		else
		{
			m_TransferCommandPool = m_GraphicsCommandPool;
		}

		return {};
	}

	[[nodiscard]] Result<> VKAPI::CreateCommandBuffers()
	{
		auto AllocateBuffers = [&](VkCommandPool pool, std::vector<VkCommandBuffer>& buffers) -> Result<>
			{
				buffers.resize(BACK_BUFFER_COUNT);
				VkCommandBufferAllocateInfo allocInfo{
					.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
					.commandPool = pool,
					.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
					.commandBufferCount = BACK_BUFFER_COUNT
				};

				VkResult vr = vkAllocateCommandBuffers(m_Device, &allocInfo, buffers.data());
				if (vr != VK_SUCCESS)
					return Unexpected(eResult::failure, std::format(L"vkAllocateCommandBuffers failed ({})", int(vr)));

				return {};
			};

		if (auto res = AllocateBuffers(m_GraphicsCommandPool, m_GraphicsCommandBuffers); !res)
			return res;

		if (m_ComputeCommandPool != m_GraphicsCommandPool)
		{
			if (auto res = AllocateBuffers(m_ComputeCommandPool, m_ComputeCommandBuffers); !res)
				return res;
		}

		if (m_TransferCommandPool != m_GraphicsCommandPool &&
			m_TransferCommandPool != m_ComputeCommandPool)
		{
			if (auto res = AllocateBuffers(m_TransferCommandPool, m_TransferCommandBuffers); !res)
				return res;
		}

		return {};
	}

	[[nodiscard]] Result<> VKAPI::CreateSyncObjects()
	{
		VkFenceCreateInfo fenceCI{
			.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
			.flags = VK_FENCE_CREATE_SIGNALED_BIT
		};

		for (size_t i = 0; i < BACK_BUFFER_COUNT; i++)
		{
			if (vkCreateFence(m_Device, &fenceCI, nullptr, &m_InFlightFences[i]) != VK_SUCCESS)
				return Unexpected(eResult::failure, std::format(L"Failed to create InFlightFence [{}]", i));
		}

		return {};
	}
#pragma endregion Initialize

#pragma region Rendering
	void VKAPI::BeginRender()
	{
		// Используем IndexFrameRender вместо IndexFrameUpdate
		uint32_t frame = m_IndexFrameRender;

		// Ждём завершения предыдущего кадра в этом слоте
		vkWaitForFences(m_Device, 1, &m_InFlightFences[frame], VK_TRUE, UINT64_MAX);

		VkCommandBuffer cmd = m_GraphicsCommandBuffers[frame];
		vkResetCommandBuffer(cmd, 0);

		VkCommandBufferBeginInfo beginInfo{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
		};
		ensure(VK_SUCCESS == vkBeginCommandBuffer(cmd, &beginInfo));
	}

	void VKAPI::SubmitCommandLists()
	{
		uint32_t frame = m_IndexFrameRender;
		VkCommandBuffer cmd = m_GraphicsCommandBuffers[frame];

		ensure(VK_SUCCESS == vkEndCommandBuffer(cmd));

		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

		VkSubmitInfo submitInfo{
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &m_CurrentWaitSemaphore,
			.pWaitDstStageMask = waitStages,
			.commandBufferCount = 1,
			.pCommandBuffers = &cmd,
			.signalSemaphoreCount = 1,
			.pSignalSemaphores = &m_CurrentSignalSemaphore
		};

		// Reset fence непосредственно перед submit
		vkResetFences(m_Device, 1, &m_InFlightFences[frame]);
		ensure(VK_SUCCESS == vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, m_InFlightFences[frame]));
	}

	void VKAPI::WaitForGpu()
	{
		vkDeviceWaitIdle(m_Device);
	}

	void VKAPI::EndRender()
	{
		IGAPI::EndRender();
	}
#pragma endregion Rendering
}
#endif // ZRENDER_API_VULKAN