#pragma once

#include "../core/IO/Path.h"
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
		Platform(const Path& path, std::shared_ptr<NativeAppData> nativeData);
		~Platform();

		[[nodiscard]] inline std::shared_ptr<NativeAppData> GetNativeData() const noexcept { return m_NativeData; }
		inline const PlatformConfig& GetPlatformConfig() const noexcept { return m_ConfigManager->GetPlatformConfig(); };

#if Z_APPLE
		static constexpr bool c_AsyncRunLoop = true;
#else
		static constexpr bool c_AsyncRunLoop = false;
#endif

	private:
		void Initialize(const Path& path);
		void InitializePlatformSpecific();
		void ShutdownPlatformSpecific();

		std::shared_ptr<NativeAppData> m_NativeData;
		std::shared_ptr<ConfigManager> m_ConfigManager;
	};
}
