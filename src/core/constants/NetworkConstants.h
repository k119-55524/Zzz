#pragma once

/**
 * @file NetworkConstants.h
 * @brief Сетевые константы и параметры подсистемы удаленного логирования.
 *
 * @details Задает параметры сокетов (localhost IPv4, дефолтный порт логгера),
 *          номер версии бинарного протокола передачи сетевых логов
 *          и размеры очереди сетевых сообщений для десктопных и мобильных платформ.
 *
 * @note Используется в:
 *       - NetworkLogger / SocketClient (отправка логов в RemoteLogViewer)
 *       - RemoteLogViewer (прием и парсинг бинарных пакетов)
 *       - NetworkService / SocketManager
 */

#include "core/CoreIncludes.h"

namespace zzz::core
{
#pragma region Network & Logger constants
	/// IPv4 адрес локального хоста
	constexpr std::string_view c_LocalhostIPv4 = "127.0.0.1";

	/// Порт по умолчанию для удаленного сетевого логгера
	constexpr zU16 c_DefaultLoggerPort = 3030;

	/// Версия бинарного протокола передачи логов по сети
	constexpr zU32 c_LogProtocolVersion = 0;

#if Z_DESKTOP
	/// Максимальный размер очереди сетевых логов на Desktop
	constexpr zU32 c_MaxNetworkLogQueueSize = 2000;
#elif Z_MOBILE
	/// Максимальный размер очереди сетевых логов на Mobile
	constexpr zU32 c_MaxNetworkLogQueueSize = 500;
#endif
#pragma endregion // Network & Logger constants
}
