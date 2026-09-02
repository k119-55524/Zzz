#pragma once

#include "core/utils/Defines.h"

#if defined(Z_METAL)

#include "engine/gapi/selectors/gpu/IGpuSelector.h"

namespace zzz::engine
{
	using namespace zzz::core;

	/**
	 * @brief Специализированный оценщик рейтинга для Metal (Заглушка).
	 */
	class MetalGpuRatingEvaluator final : public GpuRatingEvaluator
	{
	public:
		MetalGpuRatingEvaluator() = delete;

		[[nodiscard]] static zU64 CalculateScore(eGPUType type, zU64 dedicatedVramBytes) noexcept
		{
			return CalculateBaseScore(type, dedicatedVramBytes);
		}
	};

	/**
	 * @brief Специализированный селектор видеокарт для Metal (Заглушка).
	 */
	class MetalGpuSelector final : public IGpuSelector
	{
	public:
		MetalGpuSelector() = delete;
		explicit MetalGpuSelector(const std::shared_ptr<UserSettingsManager>& userSettings)
			: IGpuSelector(userSettings)
		{}
		~MetalGpuSelector() override = default;
	};
}

#endif // defined(Z_METAL)
