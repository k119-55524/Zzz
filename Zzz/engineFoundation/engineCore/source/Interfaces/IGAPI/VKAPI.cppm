
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
		VkPhysicalDevice device{};          // сам физический GPU
		uint32_t graphicsQueueFamily{};     // индекс подходящей queue family
		uint64_t score = 0;                 // вычисленный score
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
		[[nodiscard]] Result<> PickPhysicalDevice(VkSurfaceKHR surface);
		std::optional<Candidate> BestDeviceCandidat(const std::vector<VkPhysicalDevice>& devices, const VkSurfaceKHR& surface);
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
			.and_then([&]()
				{
					TestSurface_MSWin surface(m_Instance);

					if (!surface.IsValid())
						return Result<>(Unexpected(eResult::failure, L"Failed to create temp surface"));

					return PickPhysicalDevice(surface.Get());
				})
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

		uint32_t supportedVersion = VK_API_VERSION_1_0;
		// Если функция доступна (Vulkan 1.1+), получаем максимальную поддерживаемую версию API
		if (vkEnumerateInstanceVersion)
			vkEnumerateInstanceVersion(&supportedVersion);

		// Выбираем минимальную из:
		// 1) версии, поддерживаемой драйвером
		// 2) максимальной версии, которую поддерживает наш движок
		uint32_t apiVersion = std::min(supportedVersion, VULKAN_ENGINE_MAX_VERSION);

		// Логируем поддерживаемую и фактически используемую версии Vulkan
		DebugOutput(std::format(
			L"Vulkan supported: {}.{}.{} | Using: {}.{}.{}",
			VK_VERSION_MAJOR(supportedVersion), VK_VERSION_MINOR(supportedVersion), VK_VERSION_PATCH(supportedVersion),
			VK_VERSION_MAJOR(apiVersion), VK_VERSION_MINOR(apiVersion), VK_VERSION_PATCH(apiVersion)));

		// Получаем список обязательных расширений
		auto extensionsRes = GetRequiredExtensions();
		if (!extensionsRes)
			return extensionsRes.error();

		std::vector<const char*> extensions = extensionsRes.value();
		std::vector<const char*> layers;

		// Добавляем validation layer только в Debug
#ifdef _DEBUG
		// Узнаём количество доступных слоёв
		uint32_t layerCount = 0;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		// Получаем список слоёв
		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		bool validationLayerFound = false;

		// Проверяем наличие VK_LAYER_KHRONOS_validation
		for (const auto& layer : availableLayers)
		{
			if (std::strcmp(layer.layerName, "VK_LAYER_KHRONOS_validation") == 0)
			{
				validationLayerFound = true;
				break;
			}
		}

		// Если найден — добавляем в список активируемых слоёв
		if (validationLayerFound)
			layers.push_back("VK_LAYER_KHRONOS_validation");
		else
			DebugOutput(L"Warning: VK_LAYER_KHRONOS_validation not found");
#endif

		// Описание приложения для Vulkan
		VkApplicationInfo appInfo
		{
			.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
			.pApplicationName = appName,
			.applicationVersion = VK_MAKE_VERSION(appVersion.GetMajor(), appVersion.GetMinor(), appVersion.GetPatch()),
			.pEngineName = engineName,
			.engineVersion = VK_MAKE_VERSION(g_EngineVersion.GetMajor(), g_EngineVersion.GetMinor(), g_EngineVersion.GetPatch()),
			.apiVersion = apiVersion
		};

		// Структура создания VkInstance
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

		// Создаём Vulkan instance
		VkResult vr = vkCreateInstance(&ci, nullptr, &m_Instance);
		if (vr != VK_SUCCESS)
			return Unexpected(
				eResult::failure,
				std::format(L"vkCreateInstance failed ({})", int(vr)));

		// Загружаем функции уровня instance через volk
		volkLoadInstance(m_Instance);

		return {};
	}

	Result<std::vector<const char*>> VKAPI::GetRequiredExtensions()
	{
		uint32_t extCount = 0;

		// Узнаём количество доступных расширений instance
		vkEnumerateInstanceExtensionProperties(nullptr, &extCount, nullptr);

		// Получаем список доступных расширений
		std::vector<VkExtensionProperties> availableExt(extCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extCount, availableExt.data());

		// Лямбда для проверки наличия расширения в списке доступных
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

		// Лямбда для обязательного добавления расширения:
		// если расширение недоступно — возвращаем ошибку
		auto RequireExtension = [&](const char* name) -> Result<>
			{
				if (!IsExtensionAvailable(name))
					return Unexpected(
						eResult::failure,
						std::format(L"Required extension not supported: {}",
							string_to_wstring(name).value_or(L"Unknown extension name")));

				extensions.push_back(name);
				return {};
			};

		// Базовое обязательное расширение — VK_KHR_surface
		std::vector<const char*> requiredExtensions =
		{
			VK_KHR_SURFACE_EXTENSION_NAME
		};

		// Добавляем платформо-зависимое расширение (Win32, XCB, Wayland и т.д.)
		GetPlatformExtension(requiredExtensions);

#ifdef _DEBUG
		// В Debug-режиме пытаемся добавить VK_EXT_debug_utils,
		// если оно доступно
		bool debugUtilsAvailable =
			IsExtensionAvailable(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

		if (debugUtilsAvailable)
			requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

		// Проверяем и добавляем все обязательные расширения
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
		// Расширение для создания Win32 surface
		extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(ZPLATFORM_ANDROID)
		// Расширение для Android surface
		extensions.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#elif defined(ZPLATFORM_LINUX)
#if defined(USE_WAYLAND)
		// Wayland surface
		extensions.push_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
#else
		// XCB surface (X11)
		extensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#endif
#endif
	}

	[[nodiscard]] Result<> VKAPI::PickPhysicalDevice(VkSurfaceKHR surface)
	{
		// Запрашиваем количество доступных физических устройств
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr);

		if (deviceCount == 0)
			return Unexpected(eResult::failure, L"No Vulkan devices found");

		// Получаем список физических устройств
		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(m_Instance, &deviceCount, devices.data());

		// Ищем лучшее устройство среди доступных
		std::optional<Candidate> best = BestDeviceCandidat(devices, surface);
		if (!best)
			return Unexpected(eResult::failure, L"No suitable Vulkan device found");

		// Сохраняем выбранное устройство
		m_PhysicalDevice = best->device;
		m_GraphicsQueueFamily = best->graphicsQueueFamily;

		// Логируем результат выбора
		VkPhysicalDeviceProperties props{};
		vkGetPhysicalDeviceProperties(m_PhysicalDevice, &props);

		DebugOutput(std::format( L"Selected GPU: {} (score: {})", string_to_wstring(props.deviceName).value_or(L"Unknown GPU"), best->score));

		return {};
	}

	std::optional<Candidate> VKAPI::BestDeviceCandidat(const std::vector<VkPhysicalDevice>& devices, const VkSurfaceKHR& surface)
	{
		std::optional<zzz::vk::Candidate> best;

		for (VkPhysicalDevice device : devices)
		{
			uint32_t familyCount = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(device, &familyCount, nullptr);

			std::vector<VkQueueFamilyProperties> families(familyCount);
			vkGetPhysicalDeviceQueueFamilyProperties(device, &familyCount, families.data());

			std::optional<uint32_t> graphicsFamily;

			for (uint32_t i = 0; i < familyCount; ++i)
			{
				// Проверяем, поддерживает ли очередь графику
				if (families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
				{
					// Проверяем, поддерживает ли эта же очередь presentation
					// (важно для swapchain)
					VkBool32 presentSupported = VK_FALSE;
					vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupported);

					if (presentSupported)
					{
						graphicsFamily = i;
						break;
					}
				}
			}

			// Если не нашли подходящую очередь — GPU нам не подходит
			if (!graphicsFamily)
				continue;

			// ------------------------------
			// Получаем свойства устройства
			// ------------------------------
			VkPhysicalDeviceProperties props{};
			vkGetPhysicalDeviceProperties(device, &props);

			VkPhysicalDeviceMemoryProperties memProps{};
			vkGetPhysicalDeviceMemoryProperties(device, &memProps);

			// Подсчитываем объём device-local памяти (VRAM)
			uint64_t localMemory = 0;
			for (uint32_t i = 0; i < memProps.memoryHeapCount; ++i)
			{
				if (memProps.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
					localMemory += memProps.memoryHeaps[i].size;
			}

			// Проверяем обязательные features
			// Проверка фич через цепочку
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

			// TODO: переделать на применение входных требований из GAPIConfig
			// Проверка базовых фич
			if (!features2.features.samplerAnisotropy ||
				!features2.features.multiDrawIndirect ||
				!features2.features.fragmentStoresAndAtomics ||
				!features2.features.independentBlend)
				continue;

			// Проверка Vulkan 1.2 фич
			if (!features12.descriptorIndexing ||
				!features12.runtimeDescriptorArray ||
				!features12.bufferDeviceAddress ||
				!features12.timelineSemaphore)
				continue;

			// Проверка Vulkan 1.3 фич
			if (!features13.dynamicRendering)
				continue;

			// ------------------------------
			// 4. Вычисляем score устройства
			// ------------------------------

			uint64_t score = 0;

			// 4.1 Предпочитаем дискретные GPU
			// (обычно существенно мощнее интегрированных)
			if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
				score += 1'000'000;

			// 4.2 Вклад VRAM (в мегабайтах)
			// Больше VRAM — выше score
			score += localMemory / (1024ull * 1024ull);

			// 4.3 Вклад версии Vulkan API
			// Более новая версия = больше возможностей
			score += VK_VERSION_MAJOR(props.apiVersion) * 10'000;
			score += VK_VERSION_MINOR(props.apiVersion) * 1'000;

			// 4.4 Косвенный вклад по количеству queue families
			// (чем больше — тем гибче устройство)
			score += families.size() * 100;

			// ------------------------------
			// 5. Сравниваем с лучшим кандидатом
			// ------------------------------

			if (!best || score > best->score)
			{
				best = Candidate{
					.device = device,
					.graphicsQueueFamily = *graphicsFamily,
					.score = score
				};
			}
		}

		return best;
	}

	[[nodiscard]] Result<> VKAPI::CreateLogicalDevice()
	{
		float priority = 1.0f;

		VkDeviceQueueCreateInfo queueInfo
		{
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.queueFamilyIndex = m_GraphicsQueueFamily,
			.queueCount = 1,
			.pQueuePriorities = &priority
		};

		VkPhysicalDeviceFeatures features{};

		VkDeviceCreateInfo createInfo
		{
			.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
			.queueCreateInfoCount = 1,
			.pQueueCreateInfos = &queueInfo,
			.pEnabledFeatures = &features
		};

		if (vkCreateDevice(m_PhysicalDevice, &createInfo, nullptr, &m_Device) != VK_SUCCESS)
			return Unexpected(eResult::failure, L"Failed to create logical device");

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