#include "NetworkAdapterInfoCollectorMSWin.h"

#if defined(Z_WINDOWS)

#include "engine/platforms/hardware/platforms/common/WCharUtilsMSWin.h"

using namespace zzz::engine;
using namespace zzz::core;

std::vector<NetworkAdapterInfo> NetworkAdapterInfoCollectorMSWin::Collect() const
{
	std::vector<NetworkAdapterInfo> networkAdapters;

	ULONG outBufLen = 15000;
	std::vector<std::byte> adapterBuffer(outBufLen);
	PIP_ADAPTER_ADDRESSES pAddresses = reinterpret_cast<PIP_ADAPTER_ADDRESSES>(adapterBuffer.data());
	DWORD dwRetVal = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, nullptr, pAddresses, &outBufLen);

	if (dwRetVal == ERROR_BUFFER_OVERFLOW)
	{
		adapterBuffer.resize(outBufLen);
		pAddresses = reinterpret_cast<PIP_ADAPTER_ADDRESSES>(adapterBuffer.data());
		dwRetVal = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, nullptr, pAddresses, &outBufLen);
	}

	if (dwRetVal == NO_ERROR)
	{
		PIP_ADAPTER_ADDRESSES pCurrAddresses = pAddresses;
		while (pCurrAddresses != nullptr)
		{
			if (pCurrAddresses->IfType != IF_TYPE_SOFTWARE_LOOPBACK)
			{
				std::string adapterName = WCharToUtf8MSWin(pCurrAddresses->FriendlyName);

				std::string macAddress;
				if (pCurrAddresses->PhysicalAddressLength > 0)
				{
					for (DWORD i = 0; i < pCurrAddresses->PhysicalAddressLength; ++i)
					{
						if (i > 0) macAddress += "-";
						macAddress += std::format("{:02X}", static_cast<unsigned int>(pCurrAddresses->PhysicalAddress[i]));
					}
				}

				bool isActive = (pCurrAddresses->OperStatus == IfOperStatusUp);

				zU64 transmitSpeedMbps = 0;
				zU64 receiveSpeedMbps = 0;

				if (isActive && pCurrAddresses->TransmitLinkSpeed != 0 && pCurrAddresses->TransmitLinkSpeed != (zU64)-1)
				{
					transmitSpeedMbps = pCurrAddresses->TransmitLinkSpeed / (1000 * 1000);
				}

				if (isActive && pCurrAddresses->ReceiveLinkSpeed != 0 && pCurrAddresses->ReceiveLinkSpeed != (zU64)-1)
				{
					receiveSpeedMbps = pCurrAddresses->ReceiveLinkSpeed / (1000 * 1000);
				}

				networkAdapters.emplace_back(adapterName, macAddress, transmitSpeedMbps, receiveSpeedMbps, isActive);
			}
			pCurrAddresses = pCurrAddresses->Next;
		}
	}

	return networkAdapters;
}

#endif // defined(Z_WINDOWS)
