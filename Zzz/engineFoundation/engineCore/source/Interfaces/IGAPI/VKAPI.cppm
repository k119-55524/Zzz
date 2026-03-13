
export module VKAPI;

#if defined(ZRENDER_API_VULKAN)

import IGAPI;
import Ensure;
import Result;
import Version;
import StrConvert;
import GAPIConfig;
import GPUUploadVK;
import DebugOutput;
import EngineConstants;
import VKDeviceCapabilities;

using namespace zzz;
using namespace zzz::core;
using namespace zzz::logger;

namespace zzz::vk
{
#if defined(_DEBUG)
	constexpr VkObjectType GetVulkanObjectType(VkSwapchainKHR) { return VK_OBJECT_TYPE_SWAPCHAIN_KHR; }
	constexpr VkObjectType GetVulkanObjectType(VkImageView) { return VK_OBJECT_TYPE_IMAGE_VIEW; }
	constexpr VkObjectType GetVulkanObjectType(VkImage) { return VK_OBJECT_TYPE_IMAGE; }
	constexpr VkObjectType GetVulkanObjectType(VkPipeline) { return VK_OBJECT_TYPE_PIPELINE; }
	constexpr VkObjectType GetVulkanObjectType(VkRenderPass) { return VK_OBJECT_TYPE_RENDER_PASS; }
	constexpr VkObjectType GetVulkanObjectType(VkFramebuffer) { return VK_OBJECT_TYPE_FRAMEBUFFER; }
	constexpr VkObjectType GetVulkanObjectType(VkSemaphore) { return VK_OBJECT_TYPE_SEMAPHORE; }
	constexpr VkObjectType GetVulkanObjectType(VkFence) { return VK_OBJECT_TYPE_FENCE; }
	constexpr VkObjectType GetVulkanObjectType(VkDescriptorSet) { return VK_OBJECT_TYPE_DESCRIPTOR_SET; }
	constexpr VkObjectType GetVulkanObjectType(VkCommandPool) { return VK_OBJECT_TYPE_COMMAND_POOL; }
	constexpr VkObjectType GetVulkanObjectType(VkCommandBuffer) { return VK_OBJECT_TYPE_COMMAND_BUFFER; }
#endif // _DEBUG

	// Точная настройка того, что проверяется.
	struct ValidationSettings
	{
		VkBool32 fine_grained_locking{ VK_TRUE };
		VkBool32 validate_core{ VK_TRUE };
		VkBool32 check_image_layout{ VK_TRUE };
		VkBool32 check_command_buffer{ VK_TRUE };
		VkBool32 check_object_in_use{ VK_TRUE };
		VkBool32 check_query{ VK_TRUE };
		VkBool32 check_shaders{ VK_TRUE };
		VkBool32 check_shaders_caching{ VK_TRUE };
		VkBool32 unique_handles{ VK_TRUE };
		VkBool32 object_lifetime{ VK_TRUE };
		VkBool32 stateless_param{ VK_TRUE };
		std::vector<const char*> debug_action{ "VK_DBG_LAYER_ACTION_LOG_MSG" }; // "VK_DBG_LAYER_ACTION_DEBUG_OUTPUT", "VK_DBG_LAYER_ACTION_BREAK"
		std::vector<const char*> report_flags{ "error", "warn" }; // Включаем как сообщения об ошибках, так и предупреждения.

		// TODO: Пока не использую.
		// Фильтр: предупреждение о некорректной работе функции vkGetPhysicalDeviceProperties от сторонних библиотек (ImGui/VMA)
		std::vector<const char*> message_id_filter{ "WARNING-legacy-gpdp2" };

		VkBaseInStructure* buildPNextChain()
		{
			layerSettings = std::vector<VkLayerSettingEXT>
			{
				{layerName, "fine_grained_locking", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &fine_grained_locking},
				{layerName, "validate_core", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &validate_core},
				{layerName, "check_image_layout", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &check_image_layout},
				{layerName, "check_command_buffer", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &check_command_buffer},
				{layerName, "check_object_in_use", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &check_object_in_use},
				{layerName, "check_query", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &check_query},
				{layerName, "check_shaders", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &check_shaders},
				{layerName, "check_shaders_caching", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &check_shaders_caching},
				{layerName, "unique_handles", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &unique_handles},
				{layerName, "object_lifetime", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &object_lifetime},
				{layerName, "stateless_param", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &stateless_param},
				{layerName, "debug_action", VK_LAYER_SETTING_TYPE_STRING_EXT, uint32_t(debug_action.size()), debug_action.data()},
				{layerName, "report_flags", VK_LAYER_SETTING_TYPE_STRING_EXT, uint32_t(report_flags.size()), report_flags.data()},
				{layerName, "message_id_filter", VK_LAYER_SETTING_TYPE_STRING_EXT, uint32_t(message_id_filter.size()), message_id_filter.data()},
			};

			layerSettingsCreateInfo =
			{
				.sType = VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT,
				.settingCount = uint32_t(layerSettings.size()),
				.pSettings = layerSettings.data(),
			};

			return reinterpret_cast<VkBaseInStructure*>(&layerSettingsCreateInfo);
		}

		static constexpr const char* layerName{ "VK_LAYER_KHRONOS_validation" };
		std::vector<VkLayerSettingEXT> layerSettings;
		VkLayerSettingsCreateInfoEXT   layerSettingsCreateInfo{};
	};

	// Структура-кандидат для хранения лучшего устройства
	struct Candidate
	{
		VkPhysicalDevice device{};
		uint32_t graphicsQueueFamily{};
		uint32_t computeQueueFamily{};
		uint32_t transferQueueFamily{};
		uint32_t presentQueueFamily{};
		uint64_t score = 0;
		bool isCanDisableVSync = false;
		uint32_t apiVersion;
	};

	// Очередь (queue) — это последовательность команд, которые выполняются GPU строго по порядку.
	// Очередь используется для отправки (submit) командных буферов на выполнение графическим процессором.
	// FamilyIndex — индекс семейства очередей (graphics, compute, transfer и т.п.).
	// QueueIndex — индекс конкретной очереди внутри этого семейства,
	// так как в одном семействе может существовать несколько очередей.
	struct QueueInfo
	{
		uint32_t m_FamilyIndex = ~0U;	// Индекс семейства очередей (graphics / compute / transfer и др.)
		uint32_t m_QueueIndex = ~0U;	// Индекс очереди внутри семейства
		VkQueue  m_Queue{};				// Объект очереди Vulkan (дескриптор очереди GPU)
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

		//inline VkQueue GetGraphicsQueue() const noexcept { return m_GraphicsQueue; }
		//inline VkQueue GetComputeQueue() const noexcept { return m_ComputeQueue; }
		//inline VkQueue GetTransferQueue() const noexcept { return m_TransferQueue; }
		//inline VkQueue GetPresentQueue() const noexcept { return m_PresentQueue; }
		//inline uint32_t GetPresentQueueFamilyIndex() const noexcept { return m_PresentQueueFamilyIndex; }

		void SubmitCommandLists() override;
		void BeginRender() override;

		template<typename T>
		void SetDebugName(T obj, const char* name)
		{
#if defined(_DEBUG)
			ensure(m_Device, "Device cannot be null.");
			ensure(vkSetDebugUtilsObjectNameEXT, "vkSetDebugUtilsObjectNameEXT cannot be null.");

			VkDebugUtilsObjectNameInfoEXT info{};
			info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
			info.objectType = GetVulkanObjectType(obj);
			info.objectHandle = reinterpret_cast<uint64_t>(obj);
			info.pObjectName = name;

			vkSetDebugUtilsObjectNameEXT(m_Device, &info);
#endif // _DEBUG
		}

		// Простая вспомогательная программа для создания временного буфера команд,
		// используемая для записи команд загрузки данных или перехода между изображениями.
		Result<VkCommandBuffer> BeginSingleTimeCommands()
		{
			ensure(m_Device, "Device cannot be null.");
			ensure(m_TransientCmdPool, "VkCommandPool cannot be null.");

			const VkCommandBufferAllocateInfo allocInfo
			{
				.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
				.commandPool = m_TransientCmdPool,
				.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
				.commandBufferCount = 1
			};
			VkCommandBuffer cmd{};
			VkResult vr = vkAllocateCommandBuffers(m_Device, &allocInfo, &cmd);

			const VkCommandBufferBeginInfo beginInfo
			{
				.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
				.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
			};
			vr = vkBeginCommandBuffer(cmd, &beginInfo);

			return cmd;
		}

		// Отправляем временный буфер команд, ждем завершения выполнения команды и выполняем очистку.
		// Это блокирующая функция, и ее следует использовать только для небольших операций.
		Result<> EndSingleTimeCommands(VkCommandBuffer cmd)
		{
			ensure(m_Device, "Device cannot be null.");
			ensure(m_TransientCmdPool, "VkCommandPool cannot be null.");
			ensure(m_PresentQueue, "m_PresentQueue cannot be null.");

			// Отправить и очистить
			VkResult vr = vkEndCommandBuffer(cmd);

			// Создать барьер для синхронизации
			const VkFenceCreateInfo fenceInfo{ .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
			std::array<VkFence, 1>  fence{};
			vr = vkCreateFence(m_Device, &fenceInfo, nullptr, fence.data());
			if (vr != VK_SUCCESS)
				return UNEXPECTED(eResult::failure, L"Failed to vkCreateFence(): {}.", static_cast<int>(vr));

			const VkCommandBufferSubmitInfo cmdBufferInfo
			{
				.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
				.commandBuffer = cmd
			};
			const std::array<VkSubmitInfo2, 1> submitInfo
			{
				{
					{
						.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
						.commandBufferInfoCount = 1,
						.pCommandBufferInfos = &cmdBufferInfo
					}
				}
			};
			vr = vkQueueSubmit2(m_PresentQueue, uint32_t(submitInfo.size()), submitInfo.data(), fence[0]);
			if (vr != VK_SUCCESS)
				return UNEXPECTED(eResult::failure, L"Failed to vkQueueSubmit2(): {}.", static_cast<int>(vr));

			vr = vkWaitForFences(m_Device, uint32_t(fence.size()), fence.data(), VK_TRUE, UINT64_MAX);
			if (vr != VK_SUCCESS)
				return UNEXPECTED(eResult::failure, L"Failed to vkWaitForFences(): {}.", static_cast<int>(vr));

			// Очистить
			vkDestroyFence(m_Device, fence[0], nullptr);
			vkFreeCommandBuffers(m_Device, m_TransientCmdPool, 1, &cmd);

			return {};
		}

	protected:
		[[nodiscard]] Result<> Init() override;
		void WaitForGpu() override;
		void EndRender() override;

	private:
		[[nodiscard]] Result<> CreateInstance(uint32_t& apiVersion);
		[[nodiscard]] Result<std::vector<const char*>> GetRequiredExtensions();
		[[nodiscard]] const char* GetPlatformExtension();
		[[nodiscard]] Result<> CreateDebugMessenger();
		[[nodiscard]] Result<> PickPhysicalDevice(Candidate& deviceCandidate, const VkSurfaceKHR& surface);
		[[nodiscard]] std::optional<Candidate> BestDeviceCandidate(const std::vector<VkPhysicalDevice>& devices, const VkSurfaceKHR& surface);
		[[nodiscard]] Result<> CreateLogicalDevice(const Candidate& deviceCandidate);
		[[nodiscard]] Result<> CreateAllocator(uint32_t apiVersion);
		[[nodiscard]] Result<> CreatePipelineCache();
		[[nodiscard]] Result<> CreateCommandPools(const Candidate& deviceCandidate);
		[[nodiscard]] Result<> CreateCommandBuffers();

		VkInstance m_Instance{ VK_NULL_HANDLE };
		VkPhysicalDevice m_PhysicalDevice{ VK_NULL_HANDLE };
		VkDevice m_Device{ VK_NULL_HANDLE };
		std::vector<QueueInfo> m_Queues; // The queue used to submit command buffers to the GPU
		std::vector<VkExtensionProperties> m_InstanceExtensionsAvailable;
		std::vector<VkExtensionProperties> m_DeviceExtensionsAvailable;

		//uint32_t m_PresentQueueFamilyIndex;
		VkQueue m_PresentQueue{ VK_NULL_HANDLE };
		//VkQueue m_GraphicsQueue{ VK_NULL_HANDLE };
		//VkQueue m_ComputeQueue{ VK_NULL_HANDLE };
		//VkQueue m_TransferQueue{ VK_NULL_HANDLE };

		VkCommandPool m_TransferCommandPool{ VK_NULL_HANDLE };
		VkCommandPool m_TransientCmdPool{ VK_NULL_HANDLE };
		//std::vector<VkCommandBuffer> m_TransferCommandBuffers;
		//uint32_t m_GraphicsQueueFamilyIndex{ VK_QUEUE_FAMILY_IGNORED };
		//uint32_t m_TransferQueueFamilyIndex{ VK_QUEUE_FAMILY_IGNORED };
		//uint32_t m_ComputeQueueFamilyIndex{ VK_QUEUE_FAMILY_IGNORED };

		VmaAllocator m_Allocator{ VK_NULL_HANDLE };
		VkPipelineCache m_PipelineCache{ VK_NULL_HANDLE };

#if defined(_DEBUG)
		VkDebugUtilsMessengerEXT m_DebugMessenger{ VK_NULL_HANDLE };

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
		IGAPI(config, eGAPIType::Vulkan)
	{
		ensure(config, "GAPIConfig cannot be null.");
	}

	VKAPI::~VKAPI()
	{
		if (m_Device)
		{
			vkDeviceWaitIdle(m_Device);

			if (m_PipelineCache)
			{
				size_t dataSize = 0;
				vkGetPipelineCacheData(m_Device, m_PipelineCache, &dataSize, nullptr);
				std::vector<char> data(dataSize);
				vkGetPipelineCacheData(m_Device, m_PipelineCache, &dataSize, data.data());
				SavePSOCacheToDisk(data);
				vkDestroyPipelineCache(m_Device, m_PipelineCache, nullptr);
			}

			if (m_Allocator)
				vmaDestroyAllocator(m_Allocator);

			//if (m_TransferCommandPool && !m_TransferCommandBuffers.empty())
			//	vkFreeCommandBuffers(m_Device, m_TransferCommandPool, static_cast<uint32_t>(m_TransferCommandBuffers.size()), m_TransferCommandBuffers.data());

			if (m_TransferCommandPool)
				vkDestroyCommandPool(m_Device, m_TransferCommandPool, nullptr);

			if (m_TransientCmdPool)
				vkDestroyCommandPool(m_Device, m_TransientCmdPool, nullptr);

			vkDestroyDevice(m_Device, nullptr);
		}

		if (m_Instance)
		{
#if defined(_DEBUG)
			auto destroyFunc = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
				vkGetInstanceProcAddr(m_Instance, "vkDestroyDebugUtilsMessengerEXT"));

			if (destroyFunc && m_DebugMessenger)
				destroyFunc(m_Instance, m_DebugMessenger, nullptr);
#endif
			vkDestroyInstance(m_Instance, nullptr);
		}
	}

#pragma region Initialize
	[[nodiscard]] Result<> VKAPI::Init()
	{
		VkResult vr = volkInitialize();
		if (vr != VK_SUCCESS)
			return UNEXPECTED(eResult::failure, L"Volk initialize failed: {}.", static_cast<int>(vr));

		Candidate deviceCandidate;
		uint32_t apiVersion;
		Result<> res = CreateInstance(apiVersion)
			.and_then([&]() { return CreateDebugMessenger(); })
			.and_then([&]()
				{
					TestSurface_MSWin surface(m_Instance);

					if (!surface.IsValid())
						return Result<>(UNEXPECTED(eResult::failure, L"Failed to create TestSurface_MSWin surface"));

					return PickPhysicalDevice(deviceCandidate, surface.Get());
				})
			.and_then([&]() { return CreateLogicalDevice(deviceCandidate); })
			.and_then([&]() { return CreateAllocator(apiVersion); })
			.and_then([&]() { return CreatePipelineCache(); })
			.and_then([&]() { return CreateCommandPools(deviceCandidate); })
			.and_then([&]() { return CreateCommandBuffers(); })
			.and_then([&]()
				{
					m_CheckGapiSupport = safe_make_unique<VKDeviceCapabilities>(m_PhysicalDevice, m_Device);
					m_CPUtoGPUDataTransfer = safe_make_unique<GPUUploadVK>();
				});

		return res;
	}

	[[nodiscard]] Result<> VKAPI::CreateInstance(uint32_t& apiVersion)
	{
		std::string appNameStr = wstring_to_string(m_Config->GetAppName());
		std::string engineNameStr = std::string(g_EngineName);
		const char* appName = appNameStr.c_str();
		const char* engineName = engineNameStr.c_str();
		const Version& appVersion = m_Config->GetAppVersion();

		VkResult vr = vkEnumerateInstanceVersion(&apiVersion);
		if (vr != VK_SUCCESS)
			return UNEXPECTED(eResult::failure, L"Failed to vkEnumerateInstanceVersion(): {}.", static_cast<int>(vr));

		ensure(apiVersion >= VULKAN_ENGINE_MIN_VERSION, "Require Vulkan 1.4 loader");

		DOut(std::format(
			L"Vulkan supported: {}.{}.{}.",
			VK_VERSION_MAJOR(apiVersion),
			VK_VERSION_MINOR(apiVersion),
			VK_VERSION_PATCH(apiVersion)));

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
			DOut(L"Warning: VK_LAYER_KHRONOS_validation not found");
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

		ValidationSettings vs{ .validate_core = VK_TRUE };
		const VkInstanceCreateInfo instanceCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
			.pNext = vs.buildPNextChain(), // Если отладочный слой не подключён то эта настройка проигнорируется и производительность не упадёт
			.pApplicationInfo = &appInfo,
			.enabledLayerCount = static_cast<uint32_t>(layers.size()),
			.ppEnabledLayerNames = layers.empty() ? nullptr : layers.data(),
			.enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
			.ppEnabledExtensionNames = extensions.data(),
		};

		vr = vkCreateInstance(&instanceCreateInfo, nullptr, &m_Instance);
		if (vr != VK_SUCCESS)
			return UNEXPECTED(eResult::failure, L"vkCreateInstance failed ({})", static_cast<int>(vr));

		volkLoadInstance(m_Instance);

		return {};
	}

	[[nodiscard]] Result<std::vector<const char*>> VKAPI::GetRequiredExtensions()
	{
		uint32_t extCount = 0;
		auto vkRes = vkEnumerateInstanceExtensionProperties(nullptr, &extCount, nullptr);
		if (vkRes != VK_SUCCESS)
			return UNEXPECTED(eResult::failure, L"vkEnumerateInstanceExtensionProperties failed ({})", static_cast<int>(vkRes));

		std::vector<VkExtensionProperties> availableExt(extCount);
		vkRes = vkEnumerateInstanceExtensionProperties(nullptr, &extCount, availableExt.data());
		if (vkRes != VK_SUCCESS)
			return UNEXPECTED(eResult::failure, L"vkEnumerateInstanceExtensionProperties failed ({})", static_cast<int>(vkRes));

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
					return UNEXPECTED(eResult::failure, L"Required extension not supported: {}", string_to_wstring(name).value_or(L"Unknown extension name"));

				extensions.push_back(name);
				return {};
			};

		std::vector<const char*> requiredExtensions =
		{
			VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME,
			VK_KHR_SURFACE_MAINTENANCE_1_EXTENSION_NAME,
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
			return UNEXPECTED(eResult::failure, L"Failed to create debug messenger");
#endif

		return {};
	}

	[[nodiscard]] Result<> VKAPI::PickPhysicalDevice(Candidate& deviceCandidate, const VkSurfaceKHR& surface)
	{
		uint32_t deviceCount = 0;
		auto vkRes = vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr);
		if (vkRes != VK_SUCCESS)
			return UNEXPECTED(eResult::failure, L"vkEnumeratePhysicalDevices failed ({}).", static_cast<int>(vkRes));

		if (deviceCount == 0)
			return UNEXPECTED(eResult::failure, L"No Vulkan physical devices found.");

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkRes = vkEnumeratePhysicalDevices(m_Instance, &deviceCount, devices.data());
		if (vkRes != VK_SUCCESS)
			return UNEXPECTED(eResult::failure, L"vkEnumeratePhysicalDevices failed ({}).", static_cast<int>(vkRes));

		auto candidat = BestDeviceCandidate(devices, surface);
		if (!candidat)
			return UNEXPECTED(eResult::failure, L"No suitable Vulkan device found.");

		deviceCandidate = candidat.value();
		m_PhysicalDevice = deviceCandidate.device;
		m_IsCanDisableVSync = deviceCandidate.isCanDisableVSync;

		{
			VkPhysicalDeviceProperties2 props{};
			props.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
			vkGetPhysicalDeviceProperties2(m_PhysicalDevice, &props);

			std::wstring gpuName = string_to_wstring(props.properties.deviceName).value_or(L"Unknown GPU");
			uint32_t apiVersion = props.properties.apiVersion;
			uint32_t major = VK_API_VERSION_MAJOR(apiVersion);
			uint32_t minor = VK_API_VERSION_MINOR(apiVersion);
			uint32_t patch = VK_API_VERSION_PATCH(apiVersion);
			DOut(L"Selected GPU - {}(Vulkan {}.{}.{}).", gpuName, major, minor, patch);
		}

		return {};
	}

	[[nodiscard]] std::optional<Candidate> VKAPI::BestDeviceCandidate(const std::vector<VkPhysicalDevice>& devices, const VkSurfaceKHR& surface)
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
			std::optional<uint32_t> presentFamily;
			for (uint32_t i = 0; i < familyCount; ++i)
			{
				const auto& f = families[i];

				if ((f.queueFlags & VK_QUEUE_GRAPHICS_BIT) && !graphicsFamily)
					graphicsFamily = i;

				if ((f.queueFlags & VK_QUEUE_COMPUTE_BIT) &&
					!(f.queueFlags & VK_QUEUE_GRAPHICS_BIT) &&
					!computeFamily)
					computeFamily = i;

				if ((f.queueFlags & VK_QUEUE_TRANSFER_BIT) &&
					!(f.queueFlags & VK_QUEUE_GRAPHICS_BIT) &&
					!(f.queueFlags & VK_QUEUE_COMPUTE_BIT) &&
					!transferFamily)
					transferFamily = i;

				VkBool32 supportsPresent = VK_FALSE;
				vkGetPhysicalDeviceSurfaceSupportKHR(
					device,
					i,
					surface,
					&supportsPresent);

				if (supportsPresent && !presentFamily)
					presentFamily = i;
			}

			if (!computeFamily) computeFamily = graphicsFamily;
			if (!transferFamily) transferFamily = graphicsFamily;

			if (!graphicsFamily || !presentFamily)
				continue;

			VkPhysicalDeviceProperties2 props{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2 };
			vkGetPhysicalDeviceProperties2(device, &props);
			if (props.properties.apiVersion < VULKAN_ENGINE_MIN_VERSION)
				continue;

			VkPhysicalDeviceMemoryProperties memProps{};
			vkGetPhysicalDeviceMemoryProperties(device, &memProps);

			uint64_t localMemory = 0;
			for (uint32_t i = 0; i < memProps.memoryHeapCount; ++i)
			{
				if (memProps.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
					localMemory += memProps.memoryHeaps[i].size;
			}

			VkPhysicalDeviceVulkan12Features features12 { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES };
			VkPhysicalDeviceVulkan13Features features13
			{
				.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
				.pNext = &features12
			};
			VkPhysicalDeviceFeatures2 features2
			{
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

			bool isCanDisableVSync = false;  // можем ли отключить vsync?
			uint32_t presentModeCount = 0;
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
			if (presentModeCount > 0)
			{
				std::vector<VkPresentModeKHR> presentModes(presentModeCount);
				vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, presentModes.data());

				for (const auto& mode : presentModes)
				{
					if (mode == VK_PRESENT_MODE_IMMEDIATE_KHR)
					{
						isCanDisableVSync = true;  // Можем отключить vsync
						break;
					}
				}
			}

			uint64_t score = 0;
			if (props.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
				score += 1'000'000;

			score += localMemory / (1024ull * 1024ull);
			score += VK_VERSION_MAJOR(props.properties.apiVersion) * 10'000;
			score += VK_VERSION_MINOR(props.properties.apiVersion) * 1'000;
			score += families.size() * 100;

			if (!best || score > best->score)
			{
				best = Candidate{
					.device = device,
					.graphicsQueueFamily = *graphicsFamily,
					.computeQueueFamily = *computeFamily,
					.transferQueueFamily = *transferFamily,
					.presentQueueFamily = *presentFamily,
					.score = score,
					.isCanDisableVSync = isCanDisableVSync
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
		AddQueue(deviceCandidate.presentQueueFamily);

		uint32_t extCount = 0;
		auto vkRes = vkEnumerateDeviceExtensionProperties(m_PhysicalDevice, nullptr, &extCount, nullptr);
		if (vkRes != VK_SUCCESS)
			return UNEXPECTED(eResult::failure, L"vkEnumerateDeviceExtensionProperties failed ({})", static_cast<int>(vkRes));

		std::vector<VkExtensionProperties> availableExt(extCount);
		vkRes = vkEnumerateDeviceExtensionProperties(m_PhysicalDevice, nullptr, &extCount, availableExt.data());
		if (vkRes != VK_SUCCESS)
			return UNEXPECTED(eResult::failure, L"vkEnumerateDeviceExtensionProperties failed ({})", static_cast<int>(vkRes));

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
					return UNEXPECTED(eResult::failure, L"Required device extension not supported: {}", string_to_wstring(name).value_or(L"Unknown extension name"));

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
			return UNEXPECTED(eResult::failure, L"Failed to create logical device");

		//vkGetDeviceQueue(m_Device, deviceCandidate.graphicsQueueFamily, 0, &m_GraphicsQueue);
		//vkGetDeviceQueue(m_Device, deviceCandidate.computeQueueFamily, 0, &m_ComputeQueue);
		//vkGetDeviceQueue(m_Device, deviceCandidate.transferQueueFamily, 0, &m_TransferQueue);
		//vkGetDeviceQueue(m_Device, deviceCandidate.presentQueueFamily, 0, &m_PresentQueue);
		//m_GraphicsQueueFamilyIndex = deviceCandidate.graphicsQueueFamily;
		//m_TransferQueueFamilyIndex = deviceCandidate.transferQueueFamily;
		//m_ComputeQueueFamilyIndex = deviceCandidate.computeQueueFamily;
		//m_PresentQueueFamilyIndex = deviceCandidate.presentQueueFamily;

		return {};
	}

	[[nodiscard]] Result<> VKAPI::CreateAllocator(uint32_t apiVersion)
	{
		VmaVulkanFunctions vf{};
		vf.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
		vf.vkGetDeviceProcAddr = vkGetDeviceProcAddr;

		// Жёсткая проверка — чтобы не ловить assert внутри VMA
		if (!vf.vkGetInstanceProcAddr || !vf.vkGetDeviceProcAddr)
			return UNEXPECTED(eResult::failure, L"Vulkan function pointers are null");

		VmaAllocatorCreateInfo ci{};
		ci.flags = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
		ci.physicalDevice = m_PhysicalDevice;
		ci.device = m_Device;
		ci.instance = m_Instance;
		ci.vulkanApiVersion = apiVersion;
		ci.pVulkanFunctions = &vf;

		VkResult vr = vmaCreateAllocator(&ci, &m_Allocator);
		if (vr != VK_SUCCESS)
			return UNEXPECTED(eResult::failure, L"vmaCreateAllocator failed ({})", static_cast<int>(vr));

		return {};
	}

	[[nodiscard]] Result<> VKAPI::CreatePipelineCache()
	{
		std::vector<char> cacheData;

		// Пытаемся загрузить с диска
		std::ifstream file(std::filesystem::path(g_PipelineCachePath), std::ios::binary | std::ios::ate);
		if (file)
		{
			size_t size = file.tellg();
			file.seekg(0);
			cacheData.resize(size);
			file.read(cacheData.data(), size);
		}

		VkPipelineCacheCreateInfo ci{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
			.initialDataSize = cacheData.size(),
			.pInitialData = cacheData.empty() ? nullptr : cacheData.data(),
		};

		VkResult vr = vkCreatePipelineCache(m_Device, &ci, nullptr, &m_PipelineCache);
		if (vr != VK_SUCCESS)
			return UNEXPECTED(eResult::failure, L"vkCreatePipelineCache failed ({})", static_cast<int>(vr));

		return {};
	}

	[[nodiscard]] Result<> VKAPI::CreateCommandPools(const Candidate& deviceCandidate)
	{
		const bool transferIsUnique =
			deviceCandidate.transferQueueFamily != deviceCandidate.graphicsQueueFamily &&
			deviceCandidate.transferQueueFamily != deviceCandidate.computeQueueFamily;

		if (transferIsUnique)
		{
			VkCommandPoolCreateInfo ci
			{
				.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
				.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
				.queueFamilyIndex = deviceCandidate.transferQueueFamily
			};

			VkResult vr = vkCreateCommandPool(m_Device, &ci, nullptr, &m_TransferCommandPool);
			if (vr != VK_SUCCESS)
				return UNEXPECTED(eResult::failure, L"vkCreateCommandPool (transfer) failed ({})", static_cast<int>(vr));
		}

		const VkCommandPoolCreateInfo commandPoolCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT, // Hint that commands will be short-lived
			.queueFamilyIndex = deviceCandidate.transferQueueFamily,
		};
		VkResult vr = vkCreateCommandPool(m_Device, &commandPoolCreateInfo, nullptr, &m_TransientCmdPool);
		SetDebugName(m_TransientCmdPool, "Transient cmd pool");

		return {};
	}

	[[nodiscard]] Result<> VKAPI::CreateCommandBuffers()
	{
		if (m_TransferCommandPool == VK_NULL_HANDLE)
			return {}; // transfer не уникальный — буферы не нужны

		//m_TransferCommandBuffers.resize(BACK_BUFFER_COUNT);
		//VkCommandBufferAllocateInfo allocInfo{
		//	.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		//	.commandPool = m_TransferCommandPool,
		//	.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		//	.commandBufferCount = BACK_BUFFER_COUNT
		//};
		//VkResult vr = vkAllocateCommandBuffers(m_Device, &allocInfo, m_TransferCommandBuffers.data());
		//if (vr != VK_SUCCESS)
		//	return UNEXPECTED(eResult::failure, L"vkAllocateCommandBuffers (transfer) failed ({})", static_cast<int>(vr));

		return {};
	}
#pragma endregion Initialize

#pragma region Rendering
	void VKAPI::BeginRender()
	{
	}

	void VKAPI::SubmitCommandLists()
	{
	}

	void VKAPI::WaitForGpu()
	{
		if (m_Device)
			vkDeviceWaitIdle(m_Device);
	}

	void VKAPI::EndRender()
	{
		IGAPI::EndRender();
	}
#pragma endregion Rendering
}
#endif // ZRENDER_API_VULKAN