#pragma once

#include <string>
#include <string_view>
#include "core/CoreIncludes.h"

#if Z_WINDOWS
#include "core/headers/MSWin.h"
#endif

namespace zzz::engine
{
	class MonitorUtils final
	{
	public:
		MonitorUtils() = delete;

		[[nodiscard]] static inline std::string MakeId(std::string_view deviceName)
		{
#if Z_WINDOWS
			std::wstring deviceNameW(deviceName.begin(), deviceName.end());
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
			return std::string(deviceName);
#elif Z_LINUX
			return std::string(deviceName);
#elif Z_MACOS
			return std::string(deviceName);
#else
			return std::string(deviceName);
#endif
		}
	};
}
