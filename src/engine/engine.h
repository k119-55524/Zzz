#pragma once

#include <list>
#include <mutex>
#include <atomic>
#include <memory>
#include <expected>
#include <string_view>

#include "NativeAppData.h"

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


namespace zzz::engine
{
	class Engine final
	{
	public:
		Engine() = delete;
		Engine(std::string_view appName, std::shared_ptr<NativeAppData> nativeData = nullptr);
		~Engine();

		[[nodiscard]] std::expected<void, std::string> Initialize();
		[[nodiscard]] std::expected<void, std::string> Run();

		[[nodiscard]] inline std::shared_ptr<Platform> GetPlatform() const noexcept { return m_Platform; }

	private:
		void Shutdown();
		void OnUpdateSystem();

		std::mutex stateMutex;
		std::atomic<eInitState> engineState;

		std::shared_ptr<Platform> m_Platform;
		std::unique_ptr<ViewManager> m_ViewManager;
		std::shared_ptr<MainLoopBase> m_MainLoop;
	};
}
