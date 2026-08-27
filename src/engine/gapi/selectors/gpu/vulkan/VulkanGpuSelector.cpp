
#include "engine/utils/GpuUtils.h"
#include "engine/gapi/selectors/gpu/vulkan/VulkanGpuSelector.h"

#if defined(Z_VULKAN)

namespace
{
	using namespace zzz::core;
	using namespace zzz::engine;

	class VulkanGpuRatingEvaluator final : public GpuRatingEvaluator
	{
	public:
		VulkanGpuRatingEvaluator() = delete;

		[[nodiscard]] static zU64 CalculateScore(eGPUType type, zU64 dedicatedVramBytes) noexcept
		{
			return CalculateBaseScore(type, dedicatedVramBytes);
		}
	};
}

namespace zzz::engine
{
	VulkanGpuSelector::VulkanGpuSelector(const std::shared_ptr<UserSettingsManager>& userSettings)
		: IGpuSelector(userSettings)
	{
	}

	void VulkanGpuSelector::AddCandidate(VkPhysicalDevice device, VkSurfaceKHR surface)
	{
		if (!device)
			return;

		VkPhysicalDeviceProperties props{};
		vkGetPhysicalDeviceProperties(device, &props);

		VkPhysicalDeviceMemoryProperties memProps{};
		vkGetPhysicalDeviceMemoryProperties(device, &memProps);

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		uint32_t graphicsQueueFamilyIndex = UINT32_MAX;
		uint32_t presentQueueFamilyIndex = UINT32_MAX;

		for (uint32_t i = 0; i < queueFamilyCount; ++i)
		{
			if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				if (graphicsQueueFamilyIndex == UINT32_MAX)
					graphicsQueueFamilyIndex = i;
			}

			if (surface != VK_NULL_HANDLE)
			{
				VkBool32 presentSupport = VK_FALSE;
				vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
				if (presentSupport && presentQueueFamilyIndex == UINT32_MAX)
					presentQueueFamilyIndex = i;
			}
			else
			{
				if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
					presentQueueFamilyIndex = i;
			}
		}

		if (graphicsQueueFamilyIndex == UINT32_MAX)
		{
			DOutWarning("[VulkanGpuSelector::AddCandidate] GPU '{}' ignored: No Graphics Queue family.", props.deviceName);
			return;
		}

		eGPUType type = eGPUType::Integrated;
		if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			type = eGPUType::Discrete;
		else if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU)
			type = eGPUType::CpuSoftware;

		zU64 dedicatedVramBytes = 0;
		for (uint32_t i = 0; i < memProps.memoryHeapCount; ++i)
		{
			if (memProps.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
			{
				dedicatedVramBytes += memProps.memoryHeaps[i].size;
			}
		}

		std::string platformGpuId = GpuUtils::MakeId(props.vendorID, props.deviceID, 0, props.driverVersion);

		Candidate candidate{};
		candidate.device = device;
		candidate.properties = props;
		candidate.memoryProperties = memProps;
		candidate.platformGpuId = platformGpuId;
		candidate.gpuType = type;
		candidate.dedicatedVramBytes = dedicatedVramBytes;
		candidate.graphicsQueueFamilyIndex = graphicsQueueFamilyIndex;
		candidate.presentQueueFamilyIndex = presentQueueFamilyIndex;

#if Z_ANDROID
		// На мобильных платформах (Android) вычисление рейтинга пропускается — задействуется единственный SoC GPU
		candidate.score = 1000;
#else
		candidate.score = VulkanGpuRatingEvaluator::CalculateScore(type, dedicatedVramBytes);
#endif

		m_Candidates.push_back(candidate);

		DOut("[VulkanGpuSelector::AddCandidate] Added GPU Candidate: {} (Type: {}, VRAM: {} MB, Score: {})",
			props.deviceName,
			static_cast<int>(type),
			dedicatedVramBytes / (1024 * 1024),
			candidate.score
		);
	}

	VulkanGpuSelector::Candidate VulkanGpuSelector::SelectBestGpu() const
	{
		if (m_Candidates.empty())
			THROW_RUNTIME("VulkanGpuSelector: No valid GPU candidates available.");

#if Z_ANDROID
		// На Android возвращаем первого и единственного кандидата без сортировки
		return m_Candidates[0];
#else
		if (m_UserSettings)
		{
			const std::string& preferredGpuId = m_UserSettings->GetSelectedGpuId();
			if (!preferredGpuId.empty())
			{
				for (const auto& candidate : m_Candidates)
				{
					if (candidate.platformGpuId == preferredGpuId)
					{
						DOut("[VulkanGpuSelector::SelectBestGpu] Selected preferred GPU from user settings: {}", candidate.properties.deviceName);
						return candidate;
					}
				}
			}
		}

		auto bestIt = std::max_element(m_Candidates.begin(), m_Candidates.end(),
			[](const Candidate& a, const Candidate& b)
			{
				return a.score < b.score;
			});

		DOut("[VulkanGpuSelector::SelectBestGpu] Selected best GPU by score: {}", bestIt->properties.deviceName);
		return *bestIt;
#endif
	}
}

#endif // Z_VULKAN
