#pragma once

#include "../core/io/Path.h"
#include "../../NativeAppData.h"
#include "config/ConfigManager.h"
#include "config/PlatformConfig.h"

using namespace zzz::io;

namespace zzz::engine
{
	class Engine;
	
	class Platform final
	{
	public:
		Platform() = delete;
		Platform(std::string_view appName, std::shared_ptr<NativeAppData> nativeData);
		~Platform();

		inline std::string_view GetAppName() const noexcept { return m_AppName; }
		[[nodiscard]] inline std::shared_ptr<NativeAppData> GetNativeData() const noexcept { return m_NativeData; }
		inline const PlatformConfig& GetPlatformConfig() const noexcept { return m_ConfigManager->GetPlatformConfig(); };

#if Z_APPLE
		static constexpr bool c_AsyncRunLoop = true;
#else
		static constexpr bool c_AsyncRunLoop = false;
#endif		
	private:
		void Initialize();
		void InitializePlatformSpecific();
		void ShutdownPlatformSpecific();

		std::string m_AppName;
		std::shared_ptr<NativeAppData> m_NativeData;
		Path m_Path;
		std::shared_ptr<ConfigManager> m_ConfigManager;
	};
}
