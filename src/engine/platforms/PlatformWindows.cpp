#include "core/utils/Defines.h"

#if defined(Z_WINDOWS)

#include "Platform.h"
#include "window/WinMSWindows.h"
#include "engine/utils/MonitorUtils.h"
#include "core/constants/platforms/MSWinConstants.h"

Z_SET_LOG_CATEGORY(::zzz::core::Window);

using namespace zzz::engine;

void Platform::ShutdownPlatformSpecific()
{
	const auto& windowClassName = m_PlatformData.GetWindowClassName();
	const BOOL result = UnregisterClass(windowClassName.c_str(), GetModuleHandle(nullptr));
	if (!result)
	{
		const DWORD error = GetLastError();
		DOutCritical("Не удалось отменить регистрацию класса окна '{}'. Код ошибки: {}.", windowClassName, error);
	}
}

void Platform::InitializePlatformSpecific()
{
	SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

	HICON iconHandle = (HICON)LoadImage(
		GetModuleHandle(NULL),
		c_IcoResourceName.data(),
		IMAGE_ICON,
		0,
		0,
		LR_DEFAULTSIZE | LR_SHARED);

	if (!iconHandle)
		DOutWarning("Не удалось загрузить иконку '{}'. Ошибка: {}", c_IcoResourceName.data(), GetLastError());

	WNDCLASS wc = { 0 };
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WinMSWindows::WindowProc;
	wc.hInstance = GetModuleHandle(NULL);
	wc.hIcon = iconHandle;
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszClassName = m_PlatformData.GetWindowClassName().c_str();
	ATOM Result = RegisterClass(&wc);
	if (Result == 0)
		THROW_RUNTIME("Не удалось зарегистрировать класс окна. Код ошибки: {}.", GetLastError());
}

namespace
{
	BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
	{
		(void)hdcMonitor;
		(void)lprcMonitor;

		auto* monitors = reinterpret_cast<std::vector<MonitorInfo>*>(dwData);
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
			Size2D<zU32> resolution{
				static_cast<zU32>(mi.rcMonitor.right - mi.rcMonitor.left),
				static_cast<zU32>(mi.rcMonitor.bottom - mi.rcMonitor.top)
			};
			bool isPrimary = (mi.dwFlags & MONITORINFOF_PRIMARY) != 0;

			std::string platformMonitorId = MonitorUtils::MakeId(systemId);
			MonitorInfo info(platformMonitorId, systemId, resolution, mi.rcMonitor.left, mi.rcMonitor.top, isPrimary);
			monitors->push_back(info);
		}
		return TRUE;
	}
}

#endif // Z_WINDOWS
