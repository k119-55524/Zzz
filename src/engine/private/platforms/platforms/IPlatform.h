#pragma once

#include "../../core/io/Path.h"
#include "../../../platform_types.h"
#include "../../core/config/ConfigManager.h"
#include "../../core/config/platforms/IConfig.h"


using namespace zzz::io;

namespace zzz::engine
{
	class Engine;
	class EngineFactory;

	class IPlatform
	{
		friend class PlatformFactory;

	public:
		IPlatform() = delete;
		IPlatform(std::string_view appName, std::shared_ptr<PlatformNativeData> platformData);
		virtual ~IPlatform();

		inline std::string_view GetAppName() const noexcept { return m_AppName; }
		[[nodiscard]] inline std::shared_ptr<PlatformNativeData> GetPlatformData() const noexcept { return m_PlatformData; }
		inline const IConfig& GetPlatformConfig() const noexcept { return m_ConfigManager->GetPlatformConfig(); };
		inline const std::shared_ptr<EngineFactory> GetFactory() const noexcept { return m_Factory; };

		protected:
			std::string_view m_AppName;
			std::shared_ptr<PlatformNativeData> m_PlatformData;
			std::shared_ptr<Path> m_Path;
			std::shared_ptr<ConfigManager> m_ConfigManager;
			std::shared_ptr<EngineFactory> m_Factory;

		private:
			void Initialize();
			virtual void InitializeImpl() = 0;
	};
}
