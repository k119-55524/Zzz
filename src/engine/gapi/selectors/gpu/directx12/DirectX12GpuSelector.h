#pragma once

#include "engine/gapi/selectors/gpu/IGpuSelector.h"

#if defined(Z_D3D12)
#include "core/headers/DirectX12.h"

namespace zzz::engine
{
	using namespace zzz::core;

	/**
	 * @brief Специализированный оценщик рейтинга для DirectX 12.
	 * Добавляет бонусы за поддерживаемые D3D_FEATURE_LEVEL.
	 */
	class DirectX12GpuRatingEvaluator final : public GpuRatingEvaluator
	{
	public:
		DirectX12GpuRatingEvaluator() = delete;

		[[nodiscard]] static zU64 CalculateScore(eGPUType type, zU64 dedicatedVramBytes, D3D_FEATURE_LEVEL featureLevel) noexcept
		{
			const zU64 baseScore = CalculateBaseScore(type, dedicatedVramBytes);
			zU64 featureScore = 0;

			switch (featureLevel)
			{
			case D3D_FEATURE_LEVEL_12_2: // DirectX 12 Ultimate (Ray Tracing, Mesh Shaders, VRS)
				featureScore = 2000ULL;
				break;
			case D3D_FEATURE_LEVEL_12_1:
				featureScore = 1000ULL;
				break;
			case D3D_FEATURE_LEVEL_12_0:
				featureScore = 500ULL;
				break;
			default:
				featureScore = 0ULL;
				break;
			}

			return baseScore + featureScore;
		}
	};

	/**
	 * @brief Структура нативного кандидата видеокарты для DirectX 12.
	 */
	struct DirectX12GpuCandidate
	{
		Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter;
		DXGI_ADAPTER_DESC1 desc{};
		D3D_FEATURE_LEVEL maxFeatureLevel{ D3D_FEATURE_LEVEL_12_0 };
		std::string platformGpuId;
		zU64 score{ 0 };
	};

	/**
	 * @brief Специализированный селектор видеокарт для DirectX 12.
	 */
	class DirectX12GpuSelector final : public IGpuSelector
	{
	public:
		DirectX12GpuSelector() = delete;
		explicit DirectX12GpuSelector(const std::shared_ptr<UserSettingsManager>& userSettings);
		~DirectX12GpuSelector() override = default;

		/**
		 * @brief Принимает нативный DXGI-адаптер, опрашивает его характеристики и добавляет в список кандидатов.
		 */
		void AddCandidate(Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter);

		/**
		 * @brief Выполняет выбор адаптера:
		 * 1. Проверяет наличие ранее сохраненного ID в UserSettingsManager.
		 * 2. Если сохраненная карта найдена — отдает её.
		 * 3. Если не найдена или ID пуст — выбирает адаптер с максимальным Score, обновляет UserSettingsManager и сохраняет на диск.
		 */
		[[nodiscard]] Microsoft::WRL::ComPtr<IDXGIAdapter1> SelectAdapter();

		/**
		 * @brief Возвращает список всех зарегистрированных кандидатов DX12.
		 */
		[[nodiscard]] const std::vector<DirectX12GpuCandidate>& GetCandidates() const noexcept { return m_Candidates; }

	private:
		std::vector<DirectX12GpuCandidate> m_Candidates;
	};
}

#endif // Z_D3D12
