
#include "engine/gapi/selectors/gpu/IGpuSelector.h"

#if defined(Z_D3D12)
namespace zzz::engine
{
	DirectX12GpuSelector::DirectX12GpuSelector(std::shared_ptr<UserSettingsManager> userSettings)
		: IGpuSelector(std::move(userSettings))
	{
	}

	void DirectX12GpuSelector::AddCandidate(Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter)
	{
		if (!adapter)
			return;

		DXGI_ADAPTER_DESC1 desc{};
		if (FAILED(adapter->GetDesc1(&desc)))
			return;

		// Пропускаем программные эмуляторы (WARP) и невалидные нулевые адаптеры
		if ((desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) || (desc.VendorId == 0 && desc.DeviceId == 0))
			return;

		// Проверяем максимальный поддерживаемый D3D_FEATURE_LEVEL
		static constexpr D3D_FEATURE_LEVEL levelsToTest[] =
		{
			D3D_FEATURE_LEVEL_12_2,
			D3D_FEATURE_LEVEL_12_1,
			D3D_FEATURE_LEVEL_12_0
		};

		D3D_FEATURE_LEVEL maxLevel = D3D_FEATURE_LEVEL_12_0;
		bool isSupported = false;

		for (auto level : levelsToTest)
		{
			if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), level, __uuidof(ID3D12Device), nullptr)))
			{
				maxLevel = level;
				isSupported = true;
				break;
			}
		}

		if (!isSupported)
			return;

		// Формируем уникальный идентификатор platformGpuId
		std::string platformGpuId = std::format("PCI\\VEN_{:04X}&DEV_{:04X}&SUBSYS_{:08X}&REV_{:02X}",
			desc.VendorId, desc.DeviceId, desc.SubSysId, desc.Revision);

		eGPUType type = eGPUType::Integrated;
		if (desc.DedicatedVideoMemory >= 256 * 1024 * 1024)
			type = eGPUType::Discrete;

		// Расчёт рейтинга через DirectX12GpuRatingEvaluator
		const zU64 score = DirectX12GpuRatingEvaluator::CalculateScore(type, desc.DedicatedVideoMemory, maxLevel);

		DirectX12GpuCandidate candidate{};
		candidate.adapter = adapter;
		candidate.desc = desc;
		candidate.maxFeatureLevel = maxLevel;
		candidate.platformGpuId = std::move(platformGpuId);
		candidate.score = score;

		m_Candidates.push_back(std::move(candidate));
	}

	Microsoft::WRL::ComPtr<IDXGIAdapter1> DirectX12GpuSelector::SelectAdapter()
	{
		if (m_Candidates.empty())
			THROW_RUNTIME("[DirectX12GpuSelector] - Не найдено ни одного подходящего графического адаптера Direct3D 12.");

		const std::string& savedGpuId = m_UserSettings ? m_UserSettings->GetHardwareState().GetSelectedGpuId() : "";

		// 1. Проверяем, есть ли ранее сохранённый ID и присутствует ли он среди кандидатов
		if (!savedGpuId.empty())
		{
			for (const auto& candidate : m_Candidates)
			{
				if (candidate.platformGpuId == savedGpuId)
				{
					DOut("[DirectX12GpuSelector] - Выбран сохранённый адаптер: {} (VRAM: {} MB, ID: {})",
						std::string(candidate.desc.Description, candidate.desc.Description + wcslen(candidate.desc.Description)),
						candidate.desc.DedicatedVideoMemory / (1024 * 1024),
						candidate.platformGpuId);
					return candidate.adapter;
				}
			}

			DOutWarning("[DirectX12GpuSelector] - Сохранённый адаптер с ID '{}' не найден в системе. Запуск автовыбора по рейтингу...", savedGpuId);
		}

		// 2. Автовыбор по наибольшему рейтингу (Score)
		const DirectX12GpuCandidate* bestCandidate = &m_Candidates[0];
		for (std::size_t i = 1; i < m_Candidates.size(); ++i)
		{
			if (m_Candidates[i].score > bestCandidate->score)
			{
				bestCandidate = &m_Candidates[i];
			}
		}

		DOut("[DirectX12GpuSelector] - Автоматически выбран лучший адаптер по рейтингу: {} (Score: {}, VRAM: {} MB, ID: {})",
			std::string(bestCandidate->desc.Description, bestCandidate->desc.Description + wcslen(bestCandidate->desc.Description)),
			bestCandidate->score,
			bestCandidate->desc.DedicatedVideoMemory / (1024 * 1024),
			bestCandidate->platformGpuId);

		// 3. Обновляем настройки и сохраняем на диск
		if (m_UserSettings)
		{
			m_UserSettings->GetHardwareState().SetSelectedGpuId(bestCandidate->platformGpuId);
			auto saveRes = m_UserSettings->SaveConfig();
			if (!saveRes)
			{
				DOutWarning("[DirectX12GpuSelector] - Не удалось сохранить выбор GPU в конфигурацию: {}", saveRes.error());
			}
		}

		return bestCandidate->adapter;
	}
}
#endif // Z_D3D12