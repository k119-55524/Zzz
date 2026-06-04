#pragma once

#include <list>
#include <mutex>
#include <expected>
#include "headers/enums.h"
#include "private/platforms/main_loop/IMainLoop.h"
#include "private/platforms/native_view/NativeView.h"

namespace zzz::io
{
	class Path;
}

namespace zzz::engine
{
	class ConfigManager;
}

using namespace zzz;

namespace zzz::engine
{
	class Engine final
	{
	public:
		Engine() = delete;
		Engine(std::string_view appName, std::shared_ptr<void> platformData = nullptr);
		~Engine();

		[[nodiscard]] std::expected<void, std::string> Initialize(std::string_view configPath = {});
		[[nodiscard]] std::expected<void, std::string> Run();

#pragma region Mobile Lifecycle Events
#if defined(__APPLE__)
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
#endif // defined(__APPLE__)

#if defined(__ANDROID__)
		// Activity получила фокус и переходит в активное состояние.
		// Android: Activity.onResume()
		void OnPlatformActivityResumed();

		// Activity теряет фокус и переходит в неактивное состояние.
		// Android: Activity.onPause()
		void OnPlatformActivityPaused();

		// Activity становится невидимой для пользователя.
		// Android: Activity.onStop()
		void OnPlatformActivityStopped();

		// Activity снова становится видимой.
		// Android: Activity.onStart()
		void OnPlatformActivityStarted();

		// Система испытывает нехватку памяти.
		// Android: Activity.onLowMemory()
		void OnPlatformLowMemory();
#endif // defined(__ANDROID__)
#pragma endregion

	private:
		void Shutdown();

		std::string_view m_AppName;
		std::shared_ptr<void> m_PlatformData;

		std::mutex stateMutex;
		std::atomic<eInitState> engineState;

		std::shared_ptr<io::Path> m_Path;
		std::shared_ptr<ConfigManager> m_ConfigManager;
		std::list<std::shared_ptr<NativeView>> m_NativeView;
		std::shared_ptr<IMainLoop> m_MainLoop;
	};
}
