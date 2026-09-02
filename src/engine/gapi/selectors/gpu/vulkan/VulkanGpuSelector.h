#pragma once

#include "core/utils/Defines.h"

#if defined(Z_VULKAN)

#include "engine/gapi/selectors/gpu/IGpuSelector.h"

namespace zzz::engine
{
	using namespace zzz::core;

	/**
	 * @brief Селектор видеокарт для Vulkan.
	 */
	class VulkanGpuSelector final : public IGpuSelector
	{
	public:
		struct Candidate
		{
			VkPhysicalDevice device{ VK_NULL_HANDLE };
			VkPhysicalDeviceProperties properties{};
			VkPhysicalDeviceMemoryProperties memoryProperties{};
			std::string platformGpuId;
			eGPUType gpuType{ eGPUType::Unknown };
			zU64 dedicatedVramBytes{ 0 };
			zU64 score{ 0 };
			uint32_t graphicsQueueFamilyIndex{ UINT32_MAX };
			uint32_t presentQueueFamilyIndex{ UINT32_MAX };
		};

		VulkanGpuSelector() = delete;
		explicit VulkanGpuSelector(const std::shared_ptr<UserSettingsManager>& userSettings);
		~VulkanGpuSelector() override = default;

		void AddCandidate(VkPhysicalDevice device, VkSurfaceKHR surface);
		[[nodiscard]] const std::vector<Candidate>& GetCandidates() const noexcept { return m_Candidates; }
		[[nodiscard]] Candidate SelectBestGpu() const;

	private:
		std::vector<Candidate> m_Candidates;
	};
}

#endif // defined(Z_VULKAN)
