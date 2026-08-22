#pragma once

#include "IMonitorProvider.h"
#include "engine/EngineIncludes.h"

namespace zzz::engine
{
	/**
	 * @brief Платформенная реализация IMonitorProvider для мобильных операционных систем (Android / iOS).
	 * Предоставляет сведения об 1 нативном физическом дисплее смартфона/планшета.
	 */
	class MonitorProviderMobile final : public IMonitorProvider
	{
	public:
		MonitorProviderMobile()
		{
			RefreshMonitors();
		}

		~MonitorProviderMobile() override = default;

		[[nodiscard]] const std::vector<MonitorInfo>& GetMonitors() const noexcept override { return m_Monitors; }
		[[nodiscard]] MonitorInfo GetPrimaryMonitor() const override { return m_Monitors.front(); }
		[[nodiscard]] MonitorInfo GetMonitorForPoint(const Point2D<zI32>&) const override { return GetPrimaryMonitor(); }
		[[nodiscard]] MonitorInfo GetMonitorForRect(const Rect2D<zI32>&) const override { return GetPrimaryMonitor(); }
		[[nodiscard]] MonitorInfo GetMonitorById(const std::string&) const override { return GetPrimaryMonitor(); }

		[[nodiscard]] Rect2D<zI32> FitToWorkArea(const Rect2D<zI32>& /*windowRect*/, const MonitorInfo& monitor) const override
		{
			return Rect2D<zI32>{ Point2D<zI32>{0, 0}, Size2D<zI32>{static_cast<zI32>(monitor.GetResolution().GetWidth()), static_cast<zI32>(monitor.GetResolution().GetHeight())} };
		}
		[[nodiscard]] Rect2D<zI32> CenterOnWorkArea(const Rect2D<zI32>& windowRect, const MonitorInfo& monitor) const override
		{
			return FitToWorkArea(windowRect, monitor);
		}

		void RefreshMonitors() override
		{
			m_Monitors.clear();
			// Реальные физические метрики экрана смартфона/планшета (например, 2400x1080)
			m_Monitors.emplace_back("MOBILE_DISPLAY_0", "Mobile Main Display", Size2D<zU32>{1080, 1920}, 0, 0, true, 2.0f);
		}

	private:
		std::vector<MonitorInfo> m_Monitors;
	};
}
