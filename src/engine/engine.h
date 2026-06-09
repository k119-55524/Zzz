#pragma once

#include <list>
#include <mutex>
#include <atomic>
#include <memory>
#include <expected>
#include <string_view>

#include "platform_types.h"

namespace zzz
{
	enum class eInitState : zU8;
}

namespace zzz::engine
{
	class IPlatform;
	class IMainLoop;
	class NativeView;
}

using namespace zzz;

namespace zzz::engine
{
	class Engine final
	{
	public:
		Engine() = delete;
		Engine(std::string_view appName, std::shared_ptr<PlatformNativeData> platformData = nullptr);
		~Engine();

		[[nodiscard]] std::expected<void, std::string> Initialize();
		[[nodiscard]] std::expected<void, std::string> Run();

#pragma region Mobile Lifecycle Events
#if defined(Z_APPLE)
		// Приложение стало активным и может обрабатывать ввод, обновление и рендеринг.
		// iOS: applicationDidBecomeActive:
		void OnPlatformApplicationDidBecomeActive();

		// Приложение теряет активность (звонок, уведомление, переход в фон).
		// iOS: applicationWillResignActive:
		void OnPlatformApplicationWillResignActive();

		// Приложение перешло в фоновый режим.
		// Используется для сохранения состояния и пользовательских данных.
		// iOS: applicationDidEnterBackground:
		void OnPlatformApplicationDidEnterBackground();

		// Приложение начинает возвращаться из фонового режима.
		// iOS: applicationWillEnterForeground:
		void OnPlatformApplicationWillEnterForeground();

		// Система сообщает о нехватке памяти.
		// Следует освободить кэши и временные ресурсы.
		// iOS: applicationDidReceiveMemoryWarning:
		void OnPlatformApplicationDidReceiveMemoryWarning();
#endif // defined(Z_APPLE)
#pragma endregion

	private:
		void Shutdown();
		void OnUpdateSystem();
		void AddView();

		std::mutex stateMutex;
		std::atomic<eInitState> engineState;

		std::shared_ptr<IPlatform> m_Platform;
		std::list<std::shared_ptr<NativeView>> m_NativeViews;
		std::shared_ptr<IMainLoop> m_MainLoop;
	};
}
