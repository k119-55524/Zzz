#include "core/utils/Defines.h"

#if defined(Z_WINDOWS)

#include "MonitorProviderMSWin.h"

Z_SET_LOG_CATEGORY(::zzz::core::Window);

namespace
{
	using namespace zzz::core;
	using namespace zzz::math;

	struct MonitorEnumContext
	{
		std::vector<MonitorInfo>* monitors;
	};

	BOOL CALLBACK EnumMonitorsCallback(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
	{
		(void)hdcMonitor;
		(void)lprcMonitor;

		auto* ctx = reinterpret_cast<MonitorEnumContext*>(dwData);
		MONITORINFOEXW mi{};
		mi.cbSize = sizeof(MONITORINFOEXW);

		if (GetMonitorInfoW(hMonitor, &mi))
		{
			int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, mi.szDevice, -1, NULL, 0, NULL, NULL);
			std::string systemId(sizeNeeded > 1 ? sizeNeeded - 1 : 0, 0);
			if (sizeNeeded > 1)
			{
				WideCharToMultiByte(CP_UTF8, 0, mi.szDevice, -1, systemId.data(), sizeNeeded, NULL, NULL);
			}

			Size2D<zU32> logicalResolution{
				static_cast<zU32>(mi.rcMonitor.right - mi.rcMonitor.left),
				static_cast<zU32>(mi.rcMonitor.bottom - mi.rcMonitor.top)
			};

			Size2D<zU32> physicalResolution = logicalResolution;
			DEVMODEW devMode{};
			devMode.dmSize = sizeof(DEVMODEW);
			if (EnumDisplaySettingsW(mi.szDevice, ENUM_CURRENT_SETTINGS, &devMode))
			{
				if (devMode.dmPelsWidth > 0 && devMode.dmPelsHeight > 0)
				{
					physicalResolution = Size2D<zU32>{
						static_cast<zU32>(devMode.dmPelsWidth),
						static_cast<zU32>(devMode.dmPelsHeight)
					};
				}
			}

			bool isPrimary = (mi.dwFlags & MONITORINFOF_PRIMARY) != 0;
			std::string platformMonitorId = zzz::engine::MonitorUtils::MakeId(systemId);

			// Определяем системный масштаб (DPI Scale)
			UINT dpiX = 96, dpiY = 96;
			HMODULE hUser32 = GetModuleHandleW(L"user32.dll");
			if (hUser32)
			{
				typedef HRESULT(WINAPI* GetDpiForMonitorProc)(HMONITOR, int, UINT*, UINT*);
				auto getDpiProc = reinterpret_cast<GetDpiForMonitorProc>(GetProcAddress(hUser32, "GetDpiForMonitor"));
				if (getDpiProc)
				{
					getDpiProc(hMonitor, 0, &dpiX, &dpiY);
				}
			}
			float scaleFactor = static_cast<float>(dpiX) / 96.0f;
			if (scaleFactor <= 1.0001f && logicalResolution.width > 0 && physicalResolution.width > logicalResolution.width)
			{
				scaleFactor = static_cast<float>(physicalResolution.width) / static_cast<float>(logicalResolution.width);
			}

			MonitorInfo info(platformMonitorId, systemId, physicalResolution, logicalResolution, mi.rcMonitor.left, mi.rcMonitor.top, isPrimary, scaleFactor);
			ctx->monitors->push_back(info);
		}
		return TRUE;
	}
}

namespace zzz::engine
{
	MonitorProviderMSWin::MonitorProviderMSWin()
	{
		RefreshMonitors();
		LogMonitors();
	}

	void MonitorProviderMSWin::RefreshMonitors()
	{
		m_Monitors.clear();
		MonitorEnumContext ctx{ &m_Monitors };
		EnumDisplayMonitors(nullptr, nullptr, EnumMonitorsCallback, reinterpret_cast<LPARAM>(&ctx));

		if (m_Monitors.empty())
		{
			DOutWarning("[MonitorProviderMSWin] Мониторы не найдены через API. Создан первичный монитор по умолчанию 1920x1080.");
			m_Monitors.emplace_back("PRIMARY_DEFAULT", "Default Monitor", Size2D<zU32>{1920, 1080}, Size2D<zU32>{1920, 1080}, 0, 0, true, 1.0f);
		}
	}

	void MonitorProviderMSWin::LogMonitors() const
	{
#if Z_ADD_LOGGER
		DOut("[MonitorProviderMSWin] Обнаружено мониторов: {}", m_Monitors.size());
		for (std::size_t i = 0; i < m_Monitors.size(); ++i)
		{
			DOut("  Монитор #{}: ID='{}', Res={}x{}, Pos=({}, {}), Primary={}",
				i, m_Monitors[i].GetPlatformMonitorId(),
				m_Monitors[i].GetResolution().width, m_Monitors[i].GetResolution().height,
				m_Monitors[i].GetPositionX(), m_Monitors[i].GetPositionY(),
				m_Monitors[i].IsPrimary() ? "True" : "False");
		}
#endif
	}

	MonitorInfo MonitorProviderMSWin::GetPrimaryMonitor() const
	{
		for (const auto& mon : m_Monitors)
		{
			if (mon.IsPrimary())
				return mon;
		}
		return m_Monitors.empty() ? MonitorInfo("DEFAULT", "Default", Size2D<zU32>{1920, 1080}, 0, 0, true) : m_Monitors.front();
	}

	MonitorInfo MonitorProviderMSWin::GetMonitorForPoint(const Point2D<zI32>& point) const
	{
		POINT pt{ point.x, point.y };
		HMONITOR hMon = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
		MONITORINFOEXW mi{};
		mi.cbSize = sizeof(MONITORINFOEXW);

		if (GetMonitorInfoW(hMon, &mi))
		{
			int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, mi.szDevice, -1, NULL, 0, NULL, NULL);
			std::string systemId(sizeNeeded > 1 ? sizeNeeded - 1 : 0, 0);
			if (sizeNeeded > 1)
			{
				WideCharToMultiByte(CP_UTF8, 0, mi.szDevice, -1, systemId.data(), sizeNeeded, NULL, NULL);
			}
			std::string targetId = MonitorUtils::MakeId(systemId);
			for (const auto& mon : m_Monitors)
			{
				if (mon.GetPlatformMonitorId() == targetId)
					return mon;
			}
		}
		return GetPrimaryMonitor();
	}

	MonitorInfo MonitorProviderMSWin::GetMonitorForRect(const Rect2D<zI32>& rect) const
	{
		RECT r{ rect.Left(), rect.Top(), rect.Right(), rect.Bottom() };
		HMONITOR hMon = MonitorFromRect(&r, MONITOR_DEFAULTTONEAREST);
		MONITORINFOEXW mi{};
		mi.cbSize = sizeof(MONITORINFOEXW);

		if (GetMonitorInfoW(hMon, &mi))
		{
			int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, mi.szDevice, -1, NULL, 0, NULL, NULL);
			std::string systemId(sizeNeeded > 1 ? sizeNeeded - 1 : 0, 0);
			if (sizeNeeded > 1)
			{
				WideCharToMultiByte(CP_UTF8, 0, mi.szDevice, -1, systemId.data(), sizeNeeded, NULL, NULL);
			}
			std::string targetId = MonitorUtils::MakeId(systemId);
			for (const auto& mon : m_Monitors)
			{
				if (mon.GetPlatformMonitorId() == targetId)
					return mon;
			}
		}
		return GetPrimaryMonitor();
	}

	MonitorInfo MonitorProviderMSWin::GetMonitorById(const std::string& monitorId) const
	{
		if (!monitorId.empty())
		{
			for (const auto& mon : m_Monitors)
			{
				if (mon.GetPlatformMonitorId() == monitorId)
					return mon;
			}
		}
		return GetPrimaryMonitor();
	}

	namespace
	{
		RECT GetSystemWorkAreaForMonitor(const MonitorInfo& monitor)
		{
			zI32 monX = monitor.GetPositionX();
			zI32 monY = monitor.GetPositionY();
			zI32 monW = static_cast<zI32>(monitor.GetResolution().width);
			zI32 monH = static_cast<zI32>(monitor.GetResolution().height);
			RECT defaultRect{ monX, monY, monX + monW, monY + monH };

			std::wstring targetDevice(monitor.GetName().begin(), monitor.GetName().end());

			struct SearchCtx
			{
				const std::wstring* targetName;
				RECT workArea;
				bool found;
			} ctx{ &targetDevice, defaultRect, false };

			EnumDisplayMonitors(nullptr, nullptr, [](HMONITOR hMon, HDC, LPRECT, LPARAM dwData) -> BOOL {
				auto* searchCtx = reinterpret_cast<SearchCtx*>(dwData);
				MONITORINFOEXW mi{};
				mi.cbSize = sizeof(MONITORINFOEXW);
				if (GetMonitorInfoW(hMon, &mi))
				{
					if (searchCtx->targetName->empty() || searchCtx->targetName->compare(mi.szDevice) == 0)
					{
						searchCtx->workArea = mi.rcWork;
						searchCtx->found = true;
						return FALSE; // Нашли, останавливаем перебор
					}
				}
				return TRUE;
			}, reinterpret_cast<LPARAM>(&ctx));

			return ctx.workArea;
		}
	}

	Rect2D<zI32> MonitorProviderMSWin::FitToWorkArea(const Rect2D<zI32>& windowRect, const MonitorInfo& monitor) const
	{
		RECT winWorkArea = GetSystemWorkAreaForMonitor(monitor);

		zI32 workX = winWorkArea.left;
		zI32 workY = winWorkArea.top;
		zI32 workW = winWorkArea.right - winWorkArea.left;
		zI32 workH = winWorkArea.bottom - winWorkArea.top;

		zI32 targetW = std::min(static_cast<zI32>(windowRect.size.width), workW);
		zI32 targetH = std::min(static_cast<zI32>(windowRect.size.height), workH);

		zI32 targetX = windowRect.position.x;
		zI32 targetY = windowRect.position.y;

		if (targetX < workX) targetX = workX;
		if (targetY < workY) targetY = workY;
		if (targetX + targetW > workX + workW) targetX = workX + workW - targetW;
		if (targetY + targetH > workY + workH) targetY = workY + workH - targetH;

		return Rect2D<zI32>{ Point2D<zI32>{targetX, targetY}, Size2D<zI32>{targetW, targetH} };
	}

	Rect2D<zI32> MonitorProviderMSWin::CenterOnWorkArea(const Rect2D<zI32>& windowRect, const MonitorInfo& monitor) const
	{
		RECT winWorkArea = GetSystemWorkAreaForMonitor(monitor);

		zI32 workX = winWorkArea.left;
		zI32 workY = winWorkArea.top;
		zI32 workW = winWorkArea.right - winWorkArea.left;
		zI32 workH = winWorkArea.bottom - winWorkArea.top;

		zI32 targetW = std::min(static_cast<zI32>(windowRect.size.width), workW);
		zI32 targetH = std::min(static_cast<zI32>(windowRect.size.height), workH);

		zI32 targetX = workX + (workW - targetW) / 2;
		zI32 targetY = workY + (workH - targetH) / 2;

		return Rect2D<zI32>{ Point2D<zI32>{targetX, targetY}, Size2D<zI32>{targetW, targetH} };
	}
}

#endif // defined(Z_WINDOWS)
