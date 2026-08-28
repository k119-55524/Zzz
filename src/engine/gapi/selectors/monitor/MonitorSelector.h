#pragma once

#include "engine/EngineIncludes.h"
#include "engine/package/UserSettingsManager.h"
#include "core/hardware/MonitorInfo.h"

namespace zzz::engine
{
	using namespace zzz::core;

	/**
	 * @brief Селектор целевого монитора (дисплея) приложения.
	 * Накапливает кандидаты-мониторы (по аналогии с GpuSelector) и выполняет выбор.
	 */
	class MonitorSelector final
	{
	public:
		MonitorSelector() = delete;
		explicit MonitorSelector(const std::shared_ptr<UserSettingsManager>& userSettings)
			: m_UserSettings(userSettings)
		{
			ensure(m_UserSettings, "UserSettingsManager is null");
		}

		/**
		 * @brief Добавляет кандидат-монитор в список доступных выходов.
		 */
		void AddMonitor(MonitorInfo monitor)
		{
			m_Monitors.push_back(std::move(monitor));
		}

		/**
		 * @brief Выполняет выбор целевого монитора и валидацию пользовательских настроек.
		 */
		void SelectMonitor()
		{
			zU32 selectedIndex = 0;
			bool monitorFound = false;

			const auto* primaryUserData = m_UserSettings ? m_UserSettings->GetPrimaryViewUserData() : nullptr;
			const std::string savedMonitorId = primaryUserData ? primaryUserData->GetPlatformData().GetMonitorId() : "";

			if (!savedMonitorId.empty())
			{
				for (std::size_t i = 0; i < m_Monitors.size(); ++i)
				{
					if (m_Monitors[i].GetPlatformMonitorId() == savedMonitorId)
					{
						selectedIndex = static_cast<zU32>(i);
						monitorFound = true;
						DOut("[MonitorSelector] Сохраненный монитор найден: {} [#{}, ID: {}]",
							m_Monitors[i].GetName(), selectedIndex, savedMonitorId);
						break;
					}
				}
			}

			if (!monitorFound)
			{
				DOutWarning("[MonitorSelector] Сохраненный монитор с ID '{}' не найден или состав мониторов изменился.", savedMonitorId);
				
				// Ищем Primary монитор
				for (std::size_t i = 0; i < m_Monitors.size(); ++i)
				{
					if (m_Monitors[i].IsPrimary())
					{
						selectedIndex = static_cast<zU32>(i);
						break;
					}
				}

				DOut("[MonitorSelector] Выбран Primary монитор: {} [#{}, ID: {}]", m_Monitors[selectedIndex].GetName(), selectedIndex, m_Monitors[selectedIndex].GetPlatformMonitorId());
			}
		}

		/**
		 * @brief Возвращает список всех добавленных мониторов-кандидатов.
		 */
		[[nodiscard]] const std::vector<MonitorInfo>& GetMonitors() const noexcept { return m_Monitors; }

	private:
		std::shared_ptr<UserSettingsManager> m_UserSettings;
		std::vector<MonitorInfo> m_Monitors;
	};
}
