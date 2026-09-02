#pragma once

#include "core/utils/Defines.h"

#if defined(Z_LINUX)

#include "IMonitorProvider.h"
#include "engine/EngineIncludes.h"

namespace zzz::engine
{
	/**
	 * @brief Платформенная реализация IMonitorProvider для Linux (Wayland / X11).
	 */
	class MonitorProviderLinux final : public IMonitorProvider
	{
	public:
		MonitorProviderLinux()
		{
			RefreshMonitors();
		}

		~MonitorProviderLinux() override = default;

		[[nodiscard]] const std::vector<MonitorInfo>& GetMonitors() const noexcept override { return m_Monitors; }
		[[nodiscard]] MonitorInfo GetPrimaryMonitor() const override { return m_Monitors.front(); }
		[[nodiscard]] MonitorInfo GetMonitorForPoint(const Point2D<zI32>&) const override { return GetPrimaryMonitor(); }
		[[nodiscard]] MonitorInfo GetMonitorForRect(const Rect2D<zI32>&) const override { return GetPrimaryMonitor(); }
		[[nodiscard]] MonitorInfo GetMonitorById(const std::string&) const override { return GetPrimaryMonitor(); }

		[[nodiscard]] Rect2D<zI32> FitToWorkArea(const Rect2D<zI32>& /*windowRect*/, const MonitorInfo& monitor) const override
		{
			return Rect2D<zI32>{ Point2D<zI32>{0, 0}, Size2D<zI32>{static_cast<zI32>(monitor.GetResolution().width), static_cast<zI32>(monitor.GetResolution().height)} };
		}

		[[nodiscard]] Rect2D<zI32> CenterOnWorkArea(const Rect2D<zI32>& windowRect, const MonitorInfo& monitor) const override
		{
			return FitToWorkArea(windowRect, monitor);
		}

		void RefreshMonitors() override
		{
			m_Monitors.clear();
			m_Monitors.emplace_back("LINUX_DISPLAY_0", "Linux Main Display", Size2D<zU32>{1920, 1080}, 0, 0, true, 1.0f);
		}

	private:
		std::vector<MonitorInfo> m_Monitors;
	};
}

#endif // defined(Z_LINUX)
