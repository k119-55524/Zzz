#pragma once

#include <string>
#include <string_view>
#include "engine/EngineIncludes.h"

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
					int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, monitorIdW.c_str(), static_cast<int>(monitorIdW.size()), NULL, 0, NULL, NULL);
					std::string result(sizeNeeded, 0);
					WideCharToMultiByte(CP_UTF8, 0, monitorIdW.c_str(), static_cast<int>(monitorIdW.size()), result.data(), sizeNeeded, NULL, NULL);
					return result;
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
