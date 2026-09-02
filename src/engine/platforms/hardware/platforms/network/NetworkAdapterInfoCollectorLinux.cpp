#include "NetworkAdapterInfoCollectorLinux.h"

#if defined(Z_LINUX)

#include <ifaddrs.h>
#include <net/if.h>
#include <linux/if_packet.h>

using namespace zzz::engine;
using namespace zzz::core;

std::vector<NetworkAdapterInfo> NetworkAdapterInfoCollectorLinux::Collect() const
{
	std::vector<NetworkAdapterInfo> adapters;

	struct ifaddrs* ifaddr = nullptr;
	if (getifaddrs(&ifaddr) != 0)
		return adapters;

	for (struct ifaddrs* ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next)
	{
		if (!ifa->ifa_addr || ifa->ifa_addr->sa_family != AF_PACKET)
			continue;
		if (ifa->ifa_flags & IFF_LOOPBACK)
			continue;

		std::string name = ifa->ifa_name ? ifa->ifa_name : "";
		bool isActive = (ifa->ifa_flags & IFF_UP) != 0 && (ifa->ifa_flags & IFF_RUNNING) != 0;

		std::string macAddress;
		auto* sll = reinterpret_cast<struct sockaddr_ll*>(ifa->ifa_addr);
		if (sll->sll_halen == 6)
		{
			for (int i = 0; i < 6; ++i)
			{
				if (i > 0) macAddress += "-";
				macAddress += std::format("{:02X}", sll->sll_addr[i]);
			}
		}

		// Скорость линка на Linux требует отдельного ethtool-запроса (SIOCETHTOOL) - отложено до этапа
		// роллаута Linux, пока честно 0/0.
		adapters.emplace_back(name, macAddress, 0, 0, isActive);
	}

	freeifaddrs(ifaddr);
	return adapters;
}

#endif // defined(Z_LINUX)
