#include "NetworkBroadcaster.h"

#include <iostream>
#include <vector>
#include <algorithm>

#if Z_WINDOWS
	typedef int socklen_t;
#else
	#include <fcntl.h>
	#include <unistd.h>
	#include <arpa/inet.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#define SOCKET_ERROR (-1)
	#define INVALID_SOCKET (~0)
	typedef int SOCKET;
#endif

namespace zzz::logger
{
	NetworkBroadcaster::NetworkBroadcaster(std::string_view address, uint16_t port, zU32 maxQueueSize)
		: m_Address(address)
		, m_Port(port)
		, m_MaxQueueSize(maxQueueSize > 0 ? maxQueueSize : c_MaxNetworkLogQueueSize)
		, m_Socket((uint64_t)INVALID_SOCKET)
		, m_IsConnected(false)
	{
#if Z_WINDOWS
		WSADATA wsaData;
		m_WsaInitialized = (WSAStartup(MAKEWORD(2, 2), &wsaData) == 0);
		if (!m_WsaInitialized)
			return;
#endif

		m_SendThread = std::jthread([this](std::stop_token st) {
			SendThreadLoop(std::move(st));
		});
	}

	NetworkBroadcaster::~NetworkBroadcaster()
	{
		if (m_SendThread.joinable())
		{
			m_SendThread.request_stop();
			m_SendCV.notify_all();
			m_SendThread.join();
		}

		Disconnect();
#if Z_WINDOWS
		if (m_WsaInitialized)
			WSACleanup();
#endif
	}

	void NetworkBroadcaster::SetMaxQueueSize(zU32 newSize)
	{
		if (newSize == 0 || m_MaxQueueSize.load() == newSize)
			return;

		m_MaxQueueSize.store(newSize);
		m_SendCV.notify_one();
	}

	void NetworkBroadcaster::OnLog(const LogEntry& entry)
	{
		PushLogsBatch(std::span<const LogEntry>(&entry, 1));
	}

	void NetworkBroadcaster::PushLogsBatch(std::span<const LogEntry> entries)
	{
		if (entries.empty() || !m_SendThread.joinable() || m_SendThread.get_stop_token().stop_requested())
			return;

		for (const auto& entry : entries)
		{
			m_PendingLogsBuffer.Push(entry);
		}

		m_SendCV.notify_one();
	}

	void NetworkBroadcaster::SendThreadLoop(std::stop_token stopToken)
	{
		for (;;)
		{
			auto& newBatch = m_PendingLogsBuffer.SwapAndGetReadBuffer();

			if (!newBatch.empty())
			{
				m_UnsentLogs.insert(m_UnsentLogs.end(), newBatch.begin(), newBatch.end());
			}

			zU32 maxQueue = m_MaxQueueSize.load();
			if (m_UnsentLogs.size() > maxQueue)
			{
				size_t overflow = m_UnsentLogs.size() - maxQueue;
				m_UnsentLogs.erase(m_UnsentLogs.begin(), m_UnsentLogs.begin() + overflow);
			}

			if (!m_UnsentLogs.empty())
			{
				if (!m_IsConnected && stopToken.stop_requested())
				{
					break; // При закрытии приложения не пытаемся подключаться к отсутствующему серверу
				}

				Connect();
				if (m_IsConnected)
				{
					size_t sentCount = 0;
					for (const auto& entry : m_UnsentLogs)
					{
						ProcessLogSend(entry);
						if (!m_IsConnected)
							break;
						sentCount++;
					}

					if (sentCount > 0)
					{
						m_UnsentLogs.erase(m_UnsentLogs.begin(), m_UnsentLogs.begin() + sentCount);
					}
				}
			}

			if (stopToken.stop_requested())
				break;

			if (m_PendingLogsBuffer.IsEmpty() && (m_UnsentLogs.empty() || !m_IsConnected))
			{
				std::unique_lock<std::mutex> lock(m_SendMutex);
				m_SendCV.wait_for(lock, stopToken, std::chrono::milliseconds(500), [this]() {
					return !m_PendingLogsBuffer.IsEmpty();
				});
			}
		}
	}

	void NetworkBroadcaster::Connect()
	{
		if (m_IsConnected)
			return;

		auto now = std::chrono::steady_clock::now();
		if (std::chrono::duration_cast<std::chrono::milliseconds>(now - m_LastConnectAttempt).count() < 1000)
			return;

		m_LastConnectAttempt = now;

		SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (sock == (SOCKET)INVALID_SOCKET)
			return;

		sockaddr_in serverAddr{};
		serverAddr.sin_family = AF_INET;
		serverAddr.sin_port = htons(m_Port);

		if (inet_pton(AF_INET, m_Address.c_str(), &serverAddr.sin_addr) <= 0)
		{
#if Z_WINDOWS
			closesocket(sock);
#else
			close(sock);
#endif
			return;
		}

#if Z_WINDOWS
		u_long mode = 1;
		ioctlsocket(sock, FIONBIO, &mode);
#else
		int flags = fcntl(sock, F_GETFL, 0);
		if (flags == -1)
		{
			close(sock);
			return;
		}

		fcntl(sock, F_SETFL, flags | O_NONBLOCK);
#endif

		if (connect(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
		{
#if Z_WINDOWS
			if (WSAGetLastError() != WSAEWOULDBLOCK)
#else
			if (errno != EINPROGRESS)
#endif
			{
#if Z_WINDOWS
				closesocket(sock);
#else
				close(sock);
#endif
				return;
			}
		}

		fd_set writeSet;
		FD_ZERO(&writeSet);
		FD_SET(sock, &writeSet);

		timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = 50000;

		if (select((int)sock + 1, nullptr, &writeSet, nullptr, &tv) > 0)
		{
#if Z_WINDOWS
			mode = 0;
			ioctlsocket(sock, FIONBIO, &mode);
#else
			fcntl(sock, F_SETFL, flags);
#endif
			m_Socket = (uint64_t)sock;
			m_IsConnected = true;
		}
		else
		{
#if Z_WINDOWS
			closesocket(sock);
#else
			close(sock);
#endif
		}
	}

	void NetworkBroadcaster::Disconnect()
	{
		if (!m_IsConnected)
			return;

		SOCKET sock = (SOCKET)m_Socket;
#if Z_WINDOWS
		closesocket(sock);
#else
		close(sock);
#endif
		m_Socket = (uint64_t)INVALID_SOCKET;
		m_IsConnected = false;
	}

	void NetworkBroadcaster::ProcessLogSend(const LogEntry& entry)
	{
		if (!m_IsConnected)
			return;

		std::vector<std::byte> entryBytes;
		auto res = m_Serializer.Serialize(entryBytes, entry);
		if (!res)
			return;

		uint32_t size = static_cast<uint32_t>(entryBytes.size());
		std::vector<std::byte> packet;
		auto sizeRes = m_Serializer.Serialize(packet, size);
		if (!sizeRes) return;
		packet.reserve(packet.size() + entryBytes.size());
		packet.insert(packet.end(), entryBytes.begin(), entryBytes.end());

		SOCKET sock = (SOCKET)m_Socket;
		const char* data = reinterpret_cast<const char*>(packet.data());
		int totalSize = static_cast<int>(packet.size());
		int bytesSent = 0;

		while (bytesSent < totalSize)
		{
#if Z_WINDOWS
			int sent = send(sock, data + bytesSent, totalSize - bytesSent, 0);
#else
			int sent = send(sock, data + bytesSent, totalSize - bytesSent, MSG_NOSIGNAL);
#endif
			if (sent == SOCKET_ERROR)
			{
				Disconnect();
				break;
			}
			bytesSent += sent;
		}
	}
}