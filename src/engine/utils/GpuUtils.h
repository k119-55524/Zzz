#pragma once

#include <string>
#include <format>
#include "core/CoreIncludes.h"

namespace zzz::engine
{
	using namespace zzz::core;

	class GpuUtils final
	{
	public:
		GpuUtils() = delete;

		[[nodiscard]] static inline std::string MakeId(zU32 vendorId, zU32 deviceId, zU32 subSysId, zU32 revision)
		{
#if Z_WINDOWS
			return std::format("PCI\\VEN_{:04X}&DEV_{:04X}&SUBSYS_{:08X}&REV_{:02X}", vendorId, deviceId, subSysId, revision);
#elif Z_LINUX
			return std::format("PCI\\VEN_{:04X}&DEV_{:04X}&SUBSYS_{:08X}&REV_{:02X}", vendorId, deviceId, subSysId, revision);
#elif Z_MACOS
			return std::format("GPU_{:04X}_{:04X}", vendorId, deviceId);
#else
			return std::format("GPU_{:04X}_{:04X}", vendorId, deviceId);
#endif
		}
	};
}
