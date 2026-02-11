
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
	export class VKAPI final : public IGAPI
	{
		Z_NO_CREATE_COPY(VKAPI);

	public:
		explicit VKAPI(const std::shared_ptr<GAPIConfig> config);
		virtual ~VKAPI() override;

	protected:
		[[nodiscard]] Result<> Init() override;
		void WaitForGpu() override;

		void SubmitCommandLists() override;
		void BeginRender() override;
		void EndRender() override;

	private:
		[[nodiscard]] Result<> CreateInstance();
		[[nodiscard]] Result<std::vector<const char*>> GetRequiredExtensions();
		void GetPlatformExtension(std::vector<const char*>& extensions);
		[[nodiscard]] Result<> PickPhysicalDevice();
		[[nodiscard]] Result<> CreateLogicalDevice();
		[[nodiscard]] Result<> CreateCommandPool();

		VkInstance m_Instance{};
		VkPhysicalDevice m_PhysicalDevice{};
		VkDevice m_Device{};
		VkQueue m_GraphicsQueue{};
		uint32_t m_GraphicsQueueFamily{};
		VkCommandPool m_CommandPool{};

		std::shared_ptr<GAPIConfig> m_Config;
	};

	VKAPI::VKAPI(const std::shared_ptr<GAPIConfig> config) :
		IGAPI(config, eGAPIType::Vulkan),
		m_Instance(VK_NULL_HANDLE),
		m_PhysicalDevice(VK_NULL_HANDLE),
		m_Device(VK_NULL_HANDLE),
		m_GraphicsQueue(VK_NULL_HANDLE),
		m_GraphicsQueueFamily(0),
		m_CommandPool(VK_NULL_HANDLE),
		m_Config(config)
	{
		ensure(config, "GAPIConfig cannot be null.");
	}

	VKAPI::~VKAPI()
	{
		WaitForGpu();

		if (m_CommandPool)
			vkDestroyCommandPool(m_Device, m_CommandPool, nullptr);

		if (m_Device)
			vkDestroyDevice(m_Device, nullptr);

		if (m_Instance)
			vkDestroyInstance(m_Instance, nullptr);
	}

#pragma region Initialize
	[[nodiscard]] Result<> VKAPI::Init()
	{
		VkResult vr = volkInitialize();
		if (vr != VK_SUCCESS)
			return Unexpected(eResult::failure, std::format(L"volkInitialize failed ({})", int(vr)));

		Result<> res = CreateInstance()
			.and_then([&]() { return PickPhysicalDevice(); })
			.and_then([&]() { return CreateLogicalDevice(); })
			.and_then([&]() { return CreateCommandPool(); })
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

		// Определение поддерживаемой версии Vulkan
		uint32_t supportedVersion = VK_API_VERSION_1_0;

		if (vkEnumerateInstanceVersion)
			vkEnumerateInstanceVersion(&supportedVersion);

		// Выбираем минимальную из поддерживаемой bcntvjq и максимальной, которую мы хотим использовать
		uint32_t apiVersion = std::min(supportedVersion, VULKAN_ENGINE_MAX_VERSION);

		DebugOutput(std::format(
			L"Vulkan supported: {}.{}.{} | Using: {}.{}.{}",
			VK_VERSION_MAJOR(supportedVersion), VK_VERSION_MINOR(supportedVersion), VK_VERSION_PATCH(supportedVersion),
			VK_VERSION_MAJOR(apiVersion), VK_VERSION_MINOR(apiVersion), VK_VERSION_PATCH(apiVersion)));

		auto extensionsRes = GetRequiredExtensions();
		if (!extensionsRes)
			return extensionsRes.error();

		std::vector<const char*> extensions = extensionsRes.value();

		// Проверка validation layer
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

	Result<std::vector<const char*>> VKAPI::GetRequiredExtensions()
	{
		uint32_t extCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extCount, nullptr);

		std::vector<VkExtensionProperties> availableExt(extCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extCount, availableExt.data());

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

		std::vector<const char*> requiredExtensions = { VK_KHR_SURFACE_EXTENSION_NAME };
		GetPlatformExtension(requiredExtensions);

#ifdef _DEBUG
		bool debugUtilsAvailable =
			IsExtensionAvailable(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

		if (debugUtilsAvailable)
			requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

		// Проверяем и добавляем в список расширения, которые нам нужны
		for (const char* ext : requiredExtensions)
		{
			if (auto res = RequireExtension(ext); !res)
				return res.error();
		}

		return extensions;
	}

	void VKAPI::GetPlatformExtension(std::vector<const char*>& extensions)
	{
#if defined(ZPLATFORM_MSWINDOWS)
		extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(ZPLATFORM_ANDROID)
		extensions.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#elif defined(ZPLATFORM_LINUX)
#if defined(USE_WAYLAND)
		extensions.push_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
#else
		extensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#endif
#endif
	}

	[[nodiscard]] Result<> VKAPI::PickPhysicalDevice()
	{
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr);

		if (deviceCount == 0)
			return Unexpected(eResult::failure, L"No Vulkan devices found");

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(m_Instance, &deviceCount, devices.data());

		for (VkPhysicalDevice device : devices)
		{
			uint32_t familyCount = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(device, &familyCount, nullptr);

			std::vector<VkQueueFamilyProperties> families(familyCount);
			vkGetPhysicalDeviceQueueFamilyProperties(device, &familyCount, families.data());

			for (uint32_t i = 0; i < familyCount; ++i)
			{
				if (families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
				{
					m_PhysicalDevice = device;
					m_GraphicsQueueFamily = i;
					return {};
				}
			}
		}

		return Unexpected(eResult::failure, L"No suitable Vulkan device found");
	}

	[[nodiscard]] Result<> VKAPI::CreateLogicalDevice()
	{
		float priority = 1.0f;

		VkDeviceQueueCreateInfo qci
		{
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.queueFamilyIndex = m_GraphicsQueueFamily,
			.queueCount = 1,
			.pQueuePriorities = &priority
		};

		VkPhysicalDeviceFeatures features{};

		VkDeviceCreateInfo dci{
			.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
			.queueCreateInfoCount = 1,
			.pQueueCreateInfos = &qci,
			.pEnabledFeatures = &features
		};

		VkResult vr = vkCreateDevice(m_PhysicalDevice, &dci, nullptr, &m_Device);
		if (vr != VK_SUCCESS)
			return Unexpected(eResult::failure, std::format(L"vkCreateDevice failed ({})", int(vr)));

		vkGetDeviceQueue(m_Device, m_GraphicsQueueFamily, 0, &m_GraphicsQueue);

		return {};
	}

	[[nodiscard]] Result<> VKAPI::CreateCommandPool()
	{
		VkCommandPoolCreateInfo ci
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
			.queueFamilyIndex = m_GraphicsQueueFamily
		};

		VkResult vr = vkCreateCommandPool(m_Device, &ci, nullptr, &m_CommandPool);
		if (vr != VK_SUCCESS)
			return Unexpected(eResult::failure, std::format(L"vkCreateCommandPool failed ({})", int(vr)));

		return {};
	}

#pragma endregion Initialize

	void VKAPI::WaitForGpu()
	{
		if (m_Device)
			vkDeviceWaitIdle(m_Device);
	}

	void VKAPI::SubmitCommandLists()
	{
	}

	void VKAPI::BeginRender()
	{
	}

	void VKAPI::EndRender()
	{
	}
}
#endif // ZRENDER_API_VULKAN