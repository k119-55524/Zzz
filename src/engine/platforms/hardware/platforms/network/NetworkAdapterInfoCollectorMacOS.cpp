#include "NetworkAdapterInfoCollectorMacOS.h"

#if defined(Z_MACOS)

#include <ifaddrs.h>
#include <net/if.h>
#include <net/if_dl.h>

using namespace zzz::engine;
using namespace zzz::core;

std::vector<NetworkAdapterInfo> NetworkAdapterInfoCollectorMacOS::Collect() const
{
	std::vector<NetworkAdapterInfo> adapters;

	struct ifaddrs* ifaddr = nullptr;
	if (getifaddrs(&ifaddr) != 0)
		return adapters;

	for (struct ifaddrs* ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next)
	{
		if (!ifa->ifa_addr || ifa->ifa_addr->sa_family != AF_LINK)
			continue;
		if (ifa->ifa_flags & IFF_LOOPBACK)
			continue;

		std::string name = ifa->ifa_name ? ifa->ifa_name : "";
		bool isActive = (ifa->ifa_flags & IFF_UP) != 0 && (ifa->ifa_flags & IFF_RUNNING) != 0;

		std::string macAddress;
		auto* sdl = reinterpret_cast<struct sockaddr_dl*>(ifa->ifa_addr);
		if (sdl->sdl_alen == 6)
		{
			const auto* macBytes = reinterpret_cast<const unsigned char*>(LLADDR(sdl));
			for (int i = 0; i < 6; ++i)
			{
				if (i > 0) macAddress += "-";
				macAddress += std::format("{:02X}", macBytes[i]);
			}
		}

		// Скорость линка на macOS требует отдельного запроса через SIOCGIFMEDIA - отложено до этапа
		// роллаута macOS, пока честно 0/0.
		adapters.emplace_back(name, macAddress, 0, 0, isActive);
	}

	freeifaddrs(ifaddr);
	return adapters;
}

#endif // defined(Z_MACOS)
