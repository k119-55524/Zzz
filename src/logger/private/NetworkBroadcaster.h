#pragma once

#include "IBroadcaster.h"

namespace zzz::logger
{
	class NetworkBroadcaster final : public IBroadcaster
	{
	public:
		NetworkBroadcaster(std::string_view address, uint16_t port, zU32 maxQueueSize = c_MaxNetworkLogQueueSize);
		~NetworkBroadcaster() override;

		void OnLog(const LogEntry& entry) override;
		void PushLogsBatch(std::span<const LogEntry> entries) override;
		void SetMaxQueueSize(zU32 newSize) override;

	private:
		void Connect();
		void Disconnect();
		void SendThreadLoop();
		void ProcessLogSend(const LogEntry& entry);

		std::string m_Address;
		uint16_t m_Port;
		std::atomic<zU32> m_MaxQueueSize;
		uint64_t m_Socket;
		bool m_IsConnected;
		std::chrono::time_point<std::chrono::steady_clock> m_LastConnectAttempt;
		Serializer m_Serializer;

		DoubleBufferedVector<LogEntry> m_PendingLogsBuffer;
		std::vector<LogEntry> m_UnsentLogs;
		std::thread m_SendThread;
		std::condition_variable m_SendCV;
		std::mutex m_SendMutex;
		std::atomic<bool> m_SendThreadRunning{ false };

#if Z_WINDOWS
		bool m_WsaInitialized = false;
#endif
	};
}
