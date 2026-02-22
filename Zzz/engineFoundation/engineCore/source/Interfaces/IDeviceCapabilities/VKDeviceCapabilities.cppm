
export module VKDeviceCapabilities;

#if defined(ZRENDER_API_VULKAN)

import Ensure;
import StrConvert;
import DebugOutput;
import IDeviceCapabilities;

using namespace zzz;

export namespace zzz::vk
{
	export class VKDeviceCapabilities : public IDeviceCapabilities
	{
		Z_NO_COPY_MOVE(VKDeviceCapabilities);

	public:
		explicit VKDeviceCapabilities(VkPhysicalDevice physicalDevice, VkDevice device);
		~VKDeviceCapabilities() = default;

		[[nodiscard]] std::wstring GetHighestShaderModelAsString(eShaderType eShaderType) const noexcept override { return L"Vulcan doesn't need this functionality."; };

	protected:
		void CheckSupported() override;

	private:
		VkPhysicalDevice m_physicalDevice;
		VkDevice m_device;
		uint32_t m_vulkanAPIVersion = 0;
	};

	VKDeviceCapabilities::VKDeviceCapabilities(VkPhysicalDevice physicalDevice, VkDevice device) :
		m_physicalDevice(physicalDevice),
		m_device(device)
	{
		ensure(physicalDevice);
		ensure(device);
		CheckSupported();
	}

	void VKDeviceCapabilities::CheckSupported()
	{
		// --- Базовая информация о GPU ---
		VkPhysicalDeviceProperties deviceProps{};
		vkGetPhysicalDeviceProperties(m_physicalDevice, &deviceProps);
		m_vulkanAPIVersion = deviceProps.apiVersion;

		std::wstring gpuName = string_to_wstring(deviceProps.deviceName).value_or(cDefaultDeviceName);
		uint32_t vendorId = deviceProps.vendorID;
		uint32_t deviceId = deviceProps.deviceID;

		// --- Память устройства ---
		VkPhysicalDeviceMemoryProperties memProps{};
		vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memProps);

		uint64_t dedicatedVideoMemoryMB = 0;
		uint64_t sharedSystemMemoryMB = 0;

		for (uint32_t i = 0; i < memProps.memoryHeapCount; ++i)
		{
			if (memProps.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
				dedicatedVideoMemoryMB += memProps.memoryHeaps[i].size / (1024 * 1024);
			else
				sharedSystemMemoryMB += memProps.memoryHeaps[i].size / (1024 * 1024);
		}

		// --- Расширенные свойства через VkPhysicalDeviceProperties2 ---
		VkPhysicalDeviceRayTracingPipelinePropertiesKHR rtProps{};
		rtProps.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;

		VkPhysicalDeviceFragmentShadingRatePropertiesKHR vrsProps{};
		vrsProps.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_PROPERTIES_KHR;

		VkPhysicalDeviceMeshShaderPropertiesNV meshProps{};
		meshProps.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_PROPERTIES_NV;

		VkPhysicalDeviceProperties2 props2{};
		props2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
		props2.pNext = &rtProps;
		rtProps.pNext = &vrsProps;
		vrsProps.pNext = &meshProps;

		vkGetPhysicalDeviceProperties2(m_physicalDevice, &props2);

		m_RayTracingSupported = (rtProps.maxRayRecursionDepth > 0);
		m_VariableRateShadingSupported = (vrsProps.maxFragmentShadingRateAttachmentTexelSize.width > 0);
		m_MeshShadersSupported = (meshProps.maxDrawMeshTasksCount > 0);

		// Sampler Feedback через расширения
		uint32_t extCount = 0;
		vkEnumerateDeviceExtensionProperties(m_physicalDevice, nullptr, &extCount, nullptr);
		std::vector<VkExtensionProperties> extensions(extCount);
		vkEnumerateDeviceExtensionProperties(m_physicalDevice, nullptr, &extCount, extensions.data());

		m_SamplerFeedbackSupported = false;
		for (auto& ext : extensions)
		{
			if (strcmp(ext.extensionName, VK_EXT_SAMPLER_FILTER_MINMAX_EXTENSION_NAME) == 0)
				m_SamplerFeedbackSupported = true;
		}

		// --- Copy Queue и DMA ---
		m_CopyQueueSupported = true;  // упрощенно
		m_DedicatedDMASupported = true;  // упрощенно

		// --- Вычисляем "максимальную шейдерную модель" (SPIR-V) ---
		int spirvMajor = 1;
		int spirvMinor = 0;

		if (m_RayTracingSupported || m_MeshShadersSupported || m_VariableRateShadingSupported)
		{
			spirvMajor = 1;
			spirvMinor = 6; // SPIR-V 1.6 для DXR + Mesh + VRS
		}

		// Собираем строку версии SPIR-V с указанием возможностей
		std::wstring features;
		if (m_RayTracingSupported) features += L"RT+";
		if (m_MeshShadersSupported) features += L"Mesh+";
		if (m_VariableRateShadingSupported) features += L"VRS+";
		if (m_SamplerFeedbackSupported) features += L"SF+";

		if (!features.empty())
			features.pop_back(); // убрать последний '+'

		m_HighestShaderModel = std::format(L"SPIR-V {}.{}{}", spirvMajor, spirvMinor, features.empty() ? L"" : L" (" + features + L")");

		// --- Вывод полной информации ---
		DOut(std::format(
			L"Vulkan Device Capabilities:\n"
			L"     +- GPU: {}\n"
			L"     +- VendorId: 0x{:X}, DeviceId: 0x{:X}\n"
			L"     +- Dedicated Video Memory: {} MB, Shared System Memory: {} MB\n"
			L"     +- Vulkan API version: {}.{}.{}\n"
			L"     +- Hi shader model: {}\n"
			L"     +- Ray Tracing: {}\n"
			L"     +- Variable Rate Shading: {}\n"
			L"     +- Mesh Shaders: {}\n"
			L"     +- Sampler Feedback: {}\n"
			L"     +- Copy Queue: {}\n"
			L"     +- Likely Dedicated DMA: {}",
			gpuName,
			vendorId,
			deviceId,
			dedicatedVideoMemoryMB,
			sharedSystemMemoryMB,
			VK_VERSION_MAJOR(m_vulkanAPIVersion),
			VK_VERSION_MINOR(m_vulkanAPIVersion),
			VK_VERSION_PATCH(m_vulkanAPIVersion),
			m_HighestShaderModel,
			m_RayTracingSupported ? L"Yes" : L"No",
			m_VariableRateShadingSupported ? L"Yes" : L"No",
			m_MeshShadersSupported ? L"Yes" : L"No",
			m_SamplerFeedbackSupported ? L"Yes" : L"No",
			m_CopyQueueSupported ? L"Yes" : L"No",
			m_DedicatedDMASupported ? L"Yes" : L"No"
		));
	}
};
#endif // ZRENDER_API_VULKAN
