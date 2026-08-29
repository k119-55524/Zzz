#pragma once

#include "LoggerIncludes.h"
#include "private/log_entry.h"

using namespace zzz::core;

namespace zzz::logger
{
	class IBroadcaster;

	/**
	 * @brief Централизованная система логирования с поддержкой асинхронной рассылки.
	 *
	 * @details Архитектура и принципы работы подсистемы логирования:
	 *
	 * 1. **Потоковая модель**:
	 *    - **Игровой поток (Engine)**: Записывает сообщения в `DoubleBufferedVector<LogEntry>` (lock-free swap, без длительных блокировок).
	 *      Основной поток игры никогда не занимается выводом в консоль или ожиданием сетевых сокетов.
	 *    - **Поток раздатчика (Logger)**: Работает событийно (Event-driven) по `std::condition_variable`. При появлении логов
	 *      делает `swap` буферов и передает текущий пакет каждому слушателю через `IBroadcaster::PushLogsBatch`.
	 *    - **Поток сетевого бродкастера (NetworkBroadcaster)**: Имеет собственную очередь и отдельный фоновый поток I/O.
	 *      Это исключает задержки сетевого соединения на главный поток или другие бродкастеры.
	 *
	 * 2. **Категории логирования (LogCategory, см. core/utils/LogCategory.h)**:
	 *    - Каждое сообщение относится к категории - явно (`DOut(LogGAPI, "...")`) либо через ambient
	 *      `CurrentFileLogCategory` файла (см. Z_SET_LOG_CATEGORY, LogMacros.h).
	 *    - Категория проверяется рантаймом (`IsCategoryEnabled`) только для уровня `Message` (DOut) - Early Exit
	 *      происходит ещё в LogMacros.h, до форматирования строки.
	 *    - `LogWarning`, `LogError`, `LogException`, `LogCritical`, `LogFatal` - гарантированная доставка: категория
	 *      для них НЕ фильтруется рантаймом вообще (см. Z_LOG_DISPATCH, ApplyFilter=false). Каждый внешний слушатель
	 *      (напр. RemoteLogViewer) при этом может независимо скрывать категории в своих собственных View Filters -
	 *      это никак не связано с рантайм-фильтром движка.
	 *    - Категории с `isGuaranteed == true` (LogGeneral, LogEngine, а также объявленные через
	 *      Z_DECLARE_GUARANTEED_LOG_CATEGORY) никогда не фильтруются рантаймом - даже на уровне Message.
	 *    - `m_BypassAllFilters` - отладочный "рубильник", полностью отключающий фильтрацию категорий (см.
	 *      SetBypassAllFilters). Проверяется в IsCategoryEnabled первым.
	 *
	 * 3. **Правила гарантированного вывода логов**:
	 *    - `LogWarning`, `LogError`, `LogException`, `LogCritical`, `LogFatal` — обрабатываются **всегда** при активном Z_ADD_LOGGER.
	 *    - `LogMessage` — обрабатывается только при активном `Z_ADD_LOGGER` (уже включает Z_DEVELOPMENT_BUILD).
	 *    - Если взведен макрос `Z_IDE_OUT_LOGS`, лог напрямую выводится в отладочную консоль IDE (`OutputDebugString` / `__android_log`),
	 *      даже если список слушателей `m_Listeners` пуст.
	 *
	 * 4. **Управление очередью**:
	 *    - Если слушатели отсутствуют (`m_Listeners.empty()`), логи в память фонового буфера рассылки не записываются.
	 *    - Размер сетевой очереди управляется значением из `ProjectManifestData` через вызов `SetMaxNetworkLogQueueSize`.
	 */
	class Logger
	{
	public:
		Logger();
		~Logger();

		/**
		 * @brief Устанавливает маску типов логов, проходящих через систему.
		 * @param filterMask Маска фильтрации для вывода сообщений.
		 */
		static void SetLogFilterMask(eLogMessageType filterMask);

		/**
		 * @brief Проверяет, разрешена ли категория рантайм-фильтром (используется только для уровня Message, см. LogMacros.h).
		 * @details Порядок проверки: m_BypassAllFilters -> category.IsGuaranteed() -> мастер-переключатель группы
		 * (Engine/User) -> точечный opt-out список отключённых категорий по имени.
		 */
		[[nodiscard]] bool IsCategoryEnabled(const LogCategory& category) const;

		/** @brief Включает/выключает конкретную категорию по имени (рантайм; персистентность - забота вызывающего, см. UserSettingsManager). */
		void SetCategoryEnabled(std::string_view categoryName, bool enabled);

		/** @brief Мастер-переключатель: включает/выключает разом все категории группы (Engine или User). */
		void SetGroupEnabled(eLogCategoryGroup group, bool enabled) noexcept;

		/**
		 * @brief Отладочный обход всех фильтров категорий - при true IsCategoryEnabled всегда возвращает true.
		 * @details Не персистентно (сбрасывается при перезапуске); предназначено для отладки "не фильтруется ли что-то нужное".
		 */
		void SetBypassAllFilters(bool bypass) noexcept;

		/** @brief Добавляет бродкастер для вывода логов в системную консоль. */
		void AddConsoleBroadcaster();

		/**
		 * @brief Добавляет сетевой бродкастер (TCP) с собственной очередью и фоновым потоком.
		 * @param address IP-адрес приемника (по умолчанию 127.0.0.1).
		 * @param port TCP порт приемника (по умолчанию 3030).
		 * @param maxQueueSize Максимальное количество логов в изолированной очереди отправки.
		 */
		void AddNetworkBroadcaster(std::string_view address, uint16_t port, zU32 maxQueueSize = c_MaxNetworkLogQueueSize);

		/**
		 * @brief Добавляет колбэк-бродкастер для перенаправления логов во внешнюю функцию.
		 * @param callback Функция-обработчик входящих логов.
		 */
		void AddCallbackBroadcaster(LogCallback callback);

		/**
		 * @brief Останавливает и джойнит фоновый поток рассылки логов (если он запущен).
		 */
		void StopBroadcastThread();

		/**
		 * @brief Динамически обновляет размер очереди для всех зарегистрированных сетевых бродкастеров.
		 * @details Если newSize меньше текущего размера накопившейся очереди, старые логи обрезаются с головы очереди (Drop Oldest).
		 * @param newSize Новый максимальный размер очереди (игнорируется при 0).
		 */
		void SetMaxNetworkLogQueueSize(zU32 newSize);

		/** @brief Регистрирует информационное сообщение (выводится при Z_ADD_LOGGER, подлежит фильтрации по категории). */
		void LogMessage(const std::source_location& loc, const LogCategory& category, std::string formatted);
		/** @brief Регистрирует предупреждение (гарантированный вывод, категория не фильтруется). */
		void LogWarning(const std::source_location& loc, const LogCategory& category, std::string formatted);
		/** @brief Регистрирует ошибку (гарантированный вывод, категория не фильтруется). */
		void LogError(const std::source_location& loc, const LogCategory& category, std::string formatted);
		/** @brief Регистрирует исклюpение (гарантированный вывод, категория не фильтруется). */
		void LogException(const std::source_location& loc, const LogCategory& category, std::string formatted);
		/** @brief Регистрирует критическую ошибку (гарантированный вывод, категория не фильтруется). */
		void LogCritical(const std::source_location& loc, const LogCategory& category, std::string formatted);
		/** @brief Регистрирует фатальную ошибку с вызовом std::terminate() (гарантированный вывод, категория не фильтруется). */
		void LogFatal(const std::source_location& loc, const LogCategory& category, std::string formatted);

	private:
		void AddBroadcasterImpl(std::shared_ptr<IBroadcaster> broadcaster);
		void ProcessLog(const std::source_location& loc, eLogMessageType type, const LogCategory& category, std::string formatted);
		void AddToBroadcast(const std::source_location& loc, eLogMessageType type, const LogCategory& category, std::string msg);
		void StartBroadcastThreadIfNeeded();
		void BroadcastThreadLoop();
		void BroadcastLogs(const std::vector<LogEntry>& logs);

		void DebugOutputIDE(const std::source_location& loc, eLogMessageType type, const LogCategory& category, const std::string& formatted) noexcept;
		std::string MakeLogMessage(const std::source_location& loc, eLogMessageType type, const LogCategory& category, const std::string& msg);
		std::string MakeLogMessageError(const std::source_location& loc, eLogMessageType type, const LogCategory& category, const std::string& msg);
		constexpr const char* GetPlatformLogLineEnding()
		{
#if Z_LINUX || Z_ANDROID
			return "";
#else
			return "\n";
#endif
		}

		std::atomic<eLogMessageType> m_FilterMask;

		std::atomic<bool> m_BypassAllFilters{ false };
		std::atomic<bool> m_EngineGroupEnabled{ true };
		std::atomic<bool> m_UserGroupEnabled{ true };
		mutable std::mutex m_DisabledCategoriesMutex;
		std::unordered_set<std::string> m_DisabledCategories;

		std::vector<std::shared_ptr<IBroadcaster>> m_Listeners;
		std::mutex m_ListenersMutex;

		DoubleBufferedVector<LogEntry> m_LogBuffer;
		std::thread m_BroadcastThread;
		std::condition_variable m_BroadcastCV;
		std::mutex m_BroadcastMutex;
		std::atomic<bool> m_BroadcastThreadRunning{false};
	};

	inline Logger g_Logger;
}
