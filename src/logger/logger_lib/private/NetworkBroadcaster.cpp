
#include "NetworkBroadcaster.h"

#if Z_ADD_LOGGER || Z_DEVELOPMENT_BUILD

#include <iostream>
#include <vector>

#if Z_WINDOWS
	#define WIN32_LEAN_AND_MEAN
	#include <WinSock2.h>
	#include <WS2tcpip.h>
	typedef int socklen_t;
#else
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <unistd.h>
	#include <fcntl.h>
	#define SOCKET_ERROR (-1)
	#define INVALID_SOCKET (~0)
	typedef int SOCKET;
#endif

namespace zzz::logger
{
	NetworkBroadcaster::NetworkBroadcaster(std::string_view address, uint16_t port)
		: m_Address(address), m_Port(port), m_Socket((uint64_t)INVALID_SOCKET), m_IsConnected(false)
	{
#if Z_WINDOWS
		WSADATA wsaData;
		m_WsaInitialized = (WSAStartup(MAKEWORD(2, 2), &wsaData) == 0);
		if (!m_WsaInitialized)
			return;
#endif
		Connect();
	}

	NetworkBroadcaster::~NetworkBroadcaster()
	{
		Disconnect();
#if Z_WINDOWS
		if (m_WsaInitialized)
			WSACleanup();
#endif
	}

	void NetworkBroadcaster::Connect()
	{
		if (m_IsConnected)
			return;

		auto now = std::chrono::steady_clock::now();
		if (std::chrono::duration_cast<std::chrono::milliseconds>(now - m_LastConnectAttempt).count() < 1000)
			return; // Не спамим попытками подключения (раз в секунду)
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
		tv.tv_usec = 50000; // 50 ms timeout

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

	void NetworkBroadcaster::OnLog(const LogEntry& entry)
	{
		if (!m_IsConnected)
		{
			// Пытаемся переподключиться
			Connect();
			if (!m_IsConnected)
				return;
		}

		std::vector<std::byte> entryBytes;
		auto res = m_Serializer.Serialize(entryBytes, entry);
		if (!res)
			return;

		// Сначала отправляем размер, затем данные для удобного разделения пакетов по TCP
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
#endif