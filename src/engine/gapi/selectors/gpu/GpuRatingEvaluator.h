#pragma once

#include "core/CoreIncludes.h"
#include "core/hardware/GpuInfo.h"
#include "core/enums/eEnumToString.h"

using namespace zzz::core;

namespace zzz::engine
{
	/**
	 * @brief Базовый оценщик рейтинга (Score) видеокарт.
	 * Расчитывает платформонезависимый базовый балл на основе типа устройства и объёма VRAM.
	 */
	class GpuRatingEvaluator
	{
	public:
		GpuRatingEvaluator() = delete;
		virtual ~GpuRatingEvaluator() = default;

		/**
		 * @brief Вычисляет базовый балл на основе универсальной информации GpuInfo.
		 * Discrete: +10_000_000_000
		 * Integrated: +1_000_000_000
		 * CpuSoftware: +0
		 * VRAM: +1 балл за каждый МБ посвященной памяти.
		 */
		[[nodiscard]] static zU64 CalculateBaseScore(eGPUType type, zU64 dedicatedVramBytes) noexcept
		{
			zU64 typeScore = 0;
			switch (type)
			{
			case eGPUType::Discrete:
				typeScore = 10'000'000'000ULL;
				break;
			case eGPUType::Integrated:
				typeScore = 1'000'000'000ULL;
				break;
			case eGPUType::CpuSoftware:
			case eGPUType::Unknown:
			default:
				typeScore = 0ULL;
				break;
			}

			const zU64 vramMegabytes = dedicatedVramBytes / (1024ULL * 1024ULL);
			return typeScore + vramMegabytes;
		}
	};
}
