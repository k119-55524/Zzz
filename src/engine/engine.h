#pragma once

#include <mutex>
#include <atomic>
#include <memory>
#include <expected>
#include <string_view>

#include "NativeAppData.h"
#include <common/common.h>
#include <logger/logger_lib/logger.h>

namespace zzz
{
	enum class eInitState : zU8;
}

namespace zzz::engine
{
	class Platform;
	class ViewManager;
	class MainLoopBase;
}

using namespace zzz;
using namespace zzz::common;
using namespace zzz::logger;

namespace zzz::engine
{
	class Engine final
	{
	public:
		Engine() = delete;
		Engine(std::string_view appName, std::shared_ptr<NativeAppData> nativeData = nullptr);
		~Engine();

		static Engine& Get();

		[[nodiscard]] std::expected<void, std::string> Initialize();
		[[nodiscard]] std::expected<void, std::string> Run();

#pragma region Logging Configuration
		/**
		 * @brief Устанавливает маску фильтрации логов.
		 * @details Обертка над логгером. Определяет, какие типы сообщений (ошибки, ворнинги и т.д.) будут обрабатываться.
		 * @param filterMask Маска типов логов (например, eLogMessageType::All).
		 */
		inline static void SetLogFilterMask(eLogMessageType filterMask)
		{
#if Z_ADD_LOGGER || Z_DEVELOPMENT_BUILD
			Logger::SetLogFilterMask(filterMask);
#endif
		}

		/**
		 * @brief Добавляет транслятор логов в системную консоль.
		 * @details В Windows аллоцирует отдельное окно консоли. Вызывается по желанию до или после инициализации движка.
		 */
		static void AddConsoleBroadcaster()
		{
#if Z_ADD_LOGGER || Z_DEVELOPMENT_BUILD
			zzz::logger::g_Logger.AddConsoleBroadcaster();
#endif
		}
#pragma endregion

	private:
		void Shutdown();
		void OnCloseAllViews();
		void OnUpdateSystem();

		inline static Engine* s_Instance = nullptr;

		std::mutex stateMutex;
		std::atomic<eInitState> engineState;

		std::shared_ptr<Platform> m_Platform;
		std::unique_ptr<ViewManager> m_ViewManager;
		std::shared_ptr<MainLoopBase> m_MainLoop;
	};
}