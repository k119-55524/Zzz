#pragma once

#include "engine/package/UserSettingsManager.h"
#include "core/hardware/DisplayMonitorInfo.h"

namespace zzz::engine
{
	using namespace zzz::core;

	/**
	 * @brief Селектор целевого монитора (дисплея) приложения.
	 */
	class DisplayMonitorSelector final
	{
	public:
		DisplayMonitorSelector() = delete;
		explicit DisplayMonitorSelector(const std::shared_ptr<UserSettingsManager>& userSettings)
			: m_UserSettings(userSettings)
		{}

		/**
		 * @brief Принимает актуальный список мониторов системы и отдает целевой для отображения.
		 * 1. Проверяет наличие сохраненного selectedMonitorId в UserSettingsManager.
		 * 2. Если сохраненный монитор подключен — отдает его.
		 * 3. Если сохраненного нет — выбирает монитор с флагом IsPrimary(), обновляет UserSettingsManager и сохраняет на диск.
		 */
		[[nodiscard]] DisplayMonitorInfo SelectMonitor(const std::vector<DisplayMonitorInfo>& availableMonitors)
		{
			ensure(!availableMonitors.empty(), "DisplayMonitorSelector: список доступных мониторов не может быть пустым.");

			// Быстрый путь: всего 1 монитор в системе
			if (availableMonitors.size() == 1)
			{
				const auto& singleMonitor = availableMonitors[0];
				DOut("[DisplayMonitorSelector] - Единственный доступный монитор: {} ({}x{}, ID: {})",
					singleMonitor.GetName(), singleMonitor.GetResolution().width, singleMonitor.GetResolution().height, singleMonitor.GetPlatformMonitorId());

				if (m_UserSettings && m_UserSettings->GetHardwareState().GetSelectedMonitorId() != singleMonitor.GetPlatformMonitorId())
				{
					m_UserSettings->GetHardwareState().SetSelectedMonitorId(singleMonitor.GetPlatformMonitorId());
					auto saveRes = m_UserSettings->SaveConfig();
					if (!saveRes)
					{
						DOutWarning("[DisplayMonitorSelector] - Не удалось сохранить выбор монитора в конфигурацию: {}", saveRes.error());
					}
				}

				return singleMonitor;
			}

			const std::string& savedMonitorId = m_UserSettings ? m_UserSettings->GetHardwareState().GetSelectedMonitorId() : "";

			// 1. Поиск сохраненного монитора
			if (!savedMonitorId.empty())
			{
				for (const auto& monitor : availableMonitors)
				{
					if (monitor.GetPlatformMonitorId() == savedMonitorId)
					{
						DOut("[DisplayMonitorSelector] - Выбран сохраненный монитор: {} ({}x{}, Primary: {})",
							monitor.GetName(), monitor.GetResolution().width, monitor.GetResolution().height, monitor.IsPrimary());
						return monitor;
					}
				}

				DOutWarning("[DisplayMonitorSelector] - Сохраненный монитор с ID '{}' не найден. Переключение на Primary экран...", savedMonitorId);
			}

			// 2. Выбор Primary монитора (или первого попавшегося)
			const DisplayMonitorInfo* targetMonitor = &availableMonitors[0];
			for (const auto& monitor : availableMonitors)
			{
				if (monitor.IsPrimary())
				{
					targetMonitor = &monitor;
					break;
				}
			}

			DOut("[DisplayMonitorSelector] - Автоматически выбран целевой монитор: {} ({}x{}, ID: {})",
				targetMonitor->GetName(), targetMonitor->GetResolution().width, targetMonitor->GetResolution().height, targetMonitor->GetPlatformMonitorId());

			// 3. Сохранение выбранного монитора
			if (m_UserSettings)
			{
				m_UserSettings->GetHardwareState().SetSelectedMonitorId(targetMonitor->GetPlatformMonitorId());
				auto saveRes = m_UserSettings->SaveConfig();
				if (!saveRes)
				{
					DOutWarning("[DisplayMonitorSelector] - Не удалось сохранить выбор монитора в конфигурацию: {}", saveRes.error());
				}
			}

			return *targetMonitor;
		}

	private:
		std::shared_ptr<UserSettingsManager> m_UserSettings;
	};
}
