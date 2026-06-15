#pragma once

#include "IBroadcaster.h"

#if Z_ADD_LOGGER || Z_DEVELOPMENT_BUILD

#include <string>
#include <cstdint>
#include <string_view>
#include <chrono>
#include <common/serialize/Serializer.h>

using namespace zzz::common;

namespace zzz::logger
{
	class NetworkBroadcaster final : public IBroadcaster
	{
	public:
		NetworkBroadcaster(std::string_view address, uint16_t port);
		~NetworkBroadcaster() override;

		void OnLog(const LogEntry& entry) override;

	private:
		void Connect();
		void Disconnect();

		std::string m_Address;
		uint16_t m_Port;
		uint64_t m_Socket; // используем uint64_t для безопасного хранения SOCKET (Windows) и int (POSIX)
		bool m_IsConnected;
		std::chrono::time_point<std::chrono::steady_clock> m_LastConnectAttempt;
		Serializer m_Serializer;
#if Z_WINDOWS
		bool m_WsaInitialized = false;
#endif
	};
}
#endif