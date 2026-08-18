#pragma once

#include <string>
#include <string_view>
#include "core/utils/Defines.h"

#if Z_WINDOWS
#include "core/headers/MSWin.h"
#endif

namespace zzz::engine
{
	class DisplayMonitorUtils final
	{
	public:
		DisplayMonitorUtils() = delete;

		[[nodiscard]] static inline std::string MakePlatformMonitorId(std::string_view systemDeviceName)
		{
#if Z_WINDOWS
			std::wstring deviceNameW(systemDeviceName.begin(), systemDeviceName.end());
			DISPLAY_DEVICEW dd{};
			dd.cb = sizeof(DISPLAY_DEVICEW);
			if (EnumDisplayDevicesW(deviceNameW.c_str(), 0, &dd, 0))
			{
				std::wstring monitorIdW(dd.DeviceID);
				if (!monitorIdW.empty())
				{
					return std::string(monitorIdW.begin(), monitorIdW.end());
				}
			}
			return std::string(systemDeviceName);
#elif Z_LINUX
			return std::string(systemDeviceName);
#elif Z_MACOS
			return std::string(systemDeviceName);
#else
			return std::string(systemDeviceName);
#endif
		}
	};
}
