#pragma once

#include "engine/package/UserSettingsManager.h"
#include "core/hardware/DisplayMonitorInfo.h"

namespace zzz::engine
{
	using namespace zzz::core;

	/**
	 * @brief Селектор целевого монитора (дисплея) приложения.
	 * Накапливает кандидаты-мониторы (по аналогии с GpuSelector) и выполняет выбор.
	 */
	class DisplayMonitorSelector final
	{
	public:
		DisplayMonitorSelector() = delete;
		explicit DisplayMonitorSelector(const std::shared_ptr<UserSettingsManager>& userSettings)
			: m_UserSettings(userSettings)
		{}

		/**
		 * @brief Добавляет кандидат-монитор в список доступных выходов.
		 */
		void AddMonitor(DisplayMonitorInfo monitor)
		{
			m_Monitors.push_back(std::move(monitor));
		}

		/**
		 * @brief Выполняет выбор целевого монитора и валидацию пользовательских настроек.
		 */
		void SelectMonitor()
		{
			if (m_Monitors.empty())
				return;

			zU32 selectedIndex = 0;
			bool monitorFound = false;

			const std::string& savedMonitorId = m_UserSettings ? m_UserSettings->GetHardwareState().GetSelectedMonitorId() : "";

			if (!savedMonitorId.empty())
			{
				for (std::size_t i = 0; i < m_Monitors.size(); ++i)
				{
					if (m_Monitors[i].GetPlatformMonitorId() == savedMonitorId)
					{
						selectedIndex = static_cast<zU32>(i);
						monitorFound = true;
						DOut("[DisplayMonitorSelector] Сохраненный монитор найден: {} [#{}, ID: {}]",
							m_Monitors[i].GetName(), selectedIndex, savedMonitorId);
						break;
					}
				}
			}

			if (!monitorFound)
			{
				DOutWarning("[DisplayMonitorSelector] Сохраненный монитор с ID '{}' не найден или состав мониторов изменился.", savedMonitorId);
				
				// Ищем Primary монитор
				for (std::size_t i = 0; i < m_Monitors.size(); ++i)
				{
					if (m_Monitors[i].IsPrimary())
					{
						selectedIndex = static_cast<zU32>(i);
						break;
					}
				}

				DOut("[DisplayMonitorSelector] Выбран Primary монитор: {} [#{}, ID: {}]",
					m_Monitors[selectedIndex].GetName(), selectedIndex, m_Monitors[selectedIndex].GetPlatformMonitorId());

				if (m_UserSettings)
				{
					m_UserSettings->SetSelectedMonitorId(m_Monitors[selectedIndex].GetPlatformMonitorId());
					m_UserSettings->UpdateStartViewData(selectedIndex, m_Monitors);
				}
			}
			else
			{
				// Если сохраненный монитор на месте — валидируем координаты окна относительно текущего состава
				if (m_UserSettings)
				{
					m_UserSettings->UpdateStartViewData(selectedIndex, m_Monitors);
				}
			}
		}

		/**
		 * @brief Возвращает список всех добавленных мониторов-кандидатов.
		 */
		[[nodiscard]] const std::vector<DisplayMonitorInfo>& GetMonitors() const noexcept { return m_Monitors; }

	private:
		std::shared_ptr<UserSettingsManager> m_UserSettings;
		std::vector<DisplayMonitorInfo> m_Monitors;
	};
}
