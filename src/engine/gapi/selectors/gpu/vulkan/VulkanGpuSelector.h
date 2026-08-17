#pragma once

#include "engine/gapi/selectors/gpu/IGpuSelector.h"

#if defined(Z_VULKAN)

namespace zzz::engine
{
	using namespace zzz::core;

	/**
	 * @brief Специализированный оценщик рейтинга для Vulkan (Заглушка).
	 */
	class VulkanGpuRatingEvaluator final : public GpuRatingEvaluator
	{
	public:
		VulkanGpuRatingEvaluator() = delete;

		[[nodiscard]] static zU64 CalculateScore(eGPUType type, zU64 dedicatedVramBytes) noexcept
		{
			return CalculateBaseScore(type, dedicatedVramBytes);
		}
	};

	/**
	 * @brief Специализированный селектор видеокарт для Vulkan (Заглушка).
	 */
	class VulkanGpuSelector final : public IGpuSelector
	{
	public:
		VulkanGpuSelector() = delete;
		explicit VulkanGpuSelector(const std::shared_ptr<UserSettingsManager>& userSettings)
			: IGpuSelector(userSettings)
		{}
		~VulkanGpuSelector() override = default;
	};
}

#endif // Z_VULKAN
