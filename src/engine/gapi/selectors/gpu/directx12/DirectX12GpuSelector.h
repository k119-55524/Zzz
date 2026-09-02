#pragma once

#include "core/utils/Defines.h"

#if defined(Z_D3D12)

#include "engine/gapi/selectors/gpu/IGpuSelector.h"
#include "core/headers/DirectX12.h"

namespace zzz::engine
{
	using namespace zzz::core;

	/**
	 * @brief Специализированный селектор видеокарт для DirectX 12.
	 */
	class DirectX12GpuSelector final : public IGpuSelector
	{
	public:
		struct Candidate
		{
			Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter;
			DXGI_ADAPTER_DESC1 desc{};
			D3D_FEATURE_LEVEL maxFeatureLevel{ D3D_FEATURE_LEVEL_12_0 };
			zU32 outputsCount{ 0 };
			std::string platformGpuId;
			zU64 score{ 0 };
		};

		DirectX12GpuSelector() = delete;
		explicit DirectX12GpuSelector(const std::shared_ptr<UserSettingsManager>& userSettings);
		~DirectX12GpuSelector() override = default;

		/**
		 * @brief Принимает нативный DXGI-адаптер, опрашивает его характеристики и добавляет в список кандидатов.
		 */
		void AddCandidate(Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter);

		/**
		 * @brief Выполняет выбор адаптера.
		 */
		[[nodiscard]] Microsoft::WRL::ComPtr<IDXGIAdapter1> SelectAdapter();

		/**
		 * @brief Возвращает список всех зарегистрированных кандидатов DX12.
		 */
		[[nodiscard]] const std::vector<Candidate>& GetCandidates() const noexcept { return m_Candidates; }

	private:
		std::vector<Candidate> m_Candidates;
	};
}

#endif // defined(Z_D3D12)
