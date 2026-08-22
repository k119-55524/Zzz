
#include "engine/utils/GpuUtils.h"
#include "engine/gapi/selectors/gpu/directx12/DirectX12GpuSelector.h"

#if defined(Z_D3D12)

namespace zzz::engine
{
	DirectX12GpuSelector::DirectX12GpuSelector(const std::shared_ptr<UserSettingsManager>& userSettings)
		: IGpuSelector(userSettings)
	{
	}

	void DirectX12GpuSelector::AddCandidate(Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter)
	{
		if (!adapter)
			return;

		DXGI_ADAPTER_DESC1 desc{};
		if (FAILED(adapter->GetDesc1(&desc)))
			return;

		// Пропускаем только программные эмуляторы (WARP)
		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
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
		std::string platformGpuId = GpuUtils::MakeId(desc.VendorId, desc.DeviceId, desc.SubSysId, desc.Revision);

		// Избегаем дубликатов при комбинированном перечислении
		for (const auto& existing : m_Candidates)
		{
			if (existing.platformGpuId == platformGpuId)
				return;
		}

		eGPUType type = eGPUType::Integrated;
		if (desc.DedicatedVideoMemory >= 256 * 1024 * 1024)
			type = eGPUType::Discrete;

		// Подсчет количества подключенных выходов/мониторов к данному видеоадаптеру
		zU32 outputsCount = 0;
		Microsoft::WRL::ComPtr<IDXGIOutput> output;
		while (SUCCEEDED(adapter->EnumOutputs(outputsCount, &output)))
		{
			++outputsCount;
		}

		// Если к видеоадаптеру не подключено ни одного монитора (Headless), отбраковываем его
		if (outputsCount == 0)
			return;

		// Расчёт рейтинга через DirectX12GpuRatingEvaluator с учетом выходов
		const zU64 score = DirectX12GpuRatingEvaluator::CalculateScore(type, desc.DedicatedVideoMemory, maxLevel, outputsCount);

		DirectX12GpuCandidate candidate{};
		candidate.adapter = adapter;
		candidate.desc = desc;
		candidate.maxFeatureLevel = maxLevel;
		candidate.outputsCount = outputsCount;
		candidate.platformGpuId = std::move(platformGpuId);
		candidate.score = score;

		m_Candidates.push_back(std::move(candidate));
	}

	Microsoft::WRL::ComPtr<IDXGIAdapter1> DirectX12GpuSelector::SelectAdapter()
	{
		if (m_Candidates.empty())
			THROW_RUNTIME("[DirectX12GpuSelector] - Не найдено ни одного подходящего графического адаптера Direct3D 12.");

		// Быстрый путь: в системе всего один видеоадаптер (выбираем его мгновенно)
		if (m_Candidates.size() == 1)
		{
			const auto& singleCandidate = m_Candidates[0];
			const std::string gpuName = GpuUtils::GpuNameToString(singleCandidate.desc.Description);

			DOut("[DirectX12GpuSelector] - Единственный доступный видеоадаптер: {} (VRAM: {} MB, ID: {})",
				gpuName,
				singleCandidate.desc.DedicatedVideoMemory / (1024 * 1024),
				singleCandidate.platformGpuId);

			if (m_UserSettings)
			{
				m_UserSettings->SetSelectedGpuId(singleCandidate.platformGpuId);
			}

			return singleCandidate.adapter;
		}

		const std::string& savedGpuId = m_UserSettings ? m_UserSettings->GetHardwareState().GetSelectedGpuId() : "";

		// 1. Проверяем, есть ли ранее сохранённый ID и присутствует ли он среди кандидатов
		if (!savedGpuId.empty())
		{
			for (const auto& candidate : m_Candidates)
			{
				if (candidate.platformGpuId == savedGpuId)
				{
					const std::string gpuName = GpuUtils::GpuNameToString(candidate.desc.Description);

					DOut("[DirectX12GpuSelector] - Выбран сохранённый адаптер: {} (VRAM: {} MB, ID: {})",
						gpuName,
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

		// 3. Обновляем настройки
		if (m_UserSettings)
		{
			m_UserSettings->SetSelectedGpuId(bestCandidate->platformGpuId);
		}

		const std::string bestGpuName = GpuUtils::GpuNameToString(bestCandidate->desc.Description);

		DOut("[DirectX12GpuSelector] - Автоматически выбран лучший адаптер по рейтингу: {} (Score: {}, VRAM: {} MB, ID: {})",
			bestGpuName,
			bestCandidate->score,
			bestCandidate->desc.DedicatedVideoMemory / (1024 * 1024),
			bestCandidate->platformGpuId);

		return bestCandidate->adapter;
	}
}

#endif // Z_D3D12
