#pragma once

#include "IMonitorProvider.h"
#include "engine/EngineIncludes.h"
#include "engine/utils/MonitorUtils.h"

namespace zzz::engine
{
	/**
	 * @brief Платформенная реализация IMonitorProvider для операционных систем Windows (MSWin).
	 */
	class MonitorProviderMSWin final : public IMonitorProvider
	{
	public:
		MonitorProviderMSWin();
		~MonitorProviderMSWin() override = default;

		[[nodiscard]] const std::vector<MonitorInfo>& GetMonitors() const noexcept override { return m_Monitors; }
		[[nodiscard]] MonitorInfo GetPrimaryMonitor() const override;
		[[nodiscard]] MonitorInfo GetMonitorForPoint(const Point2D<zI32>& point) const override;
		[[nodiscard]] MonitorInfo GetMonitorForRect(const Rect2D<zI32>& rect) const override;
		[[nodiscard]] MonitorInfo GetMonitorById(const std::string& monitorId) const override;
		[[nodiscard]] Rect2D<zI32> FitToWorkArea(const Rect2D<zI32>& windowRect, const MonitorInfo& monitor) const override;
		[[nodiscard]] Rect2D<zI32> CenterOnWorkArea(const Rect2D<zI32>& windowRect, const MonitorInfo& monitor) const override;
		void RefreshMonitors() override;

	private:
		void LogMonitors() const;
		std::vector<MonitorInfo> m_Monitors;
	};
}
