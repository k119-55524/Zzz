#pragma once

#include "../../core/io/Path.h"
#include "../../core/config/ConfigManager.h"

using namespace zzz::io;

namespace zzz::engine
{
	class Engine;
	class PlatformFactory;

	class IPlatform
	{
		friend class Engine;

	public:
		IPlatform() = delete;
		IPlatform(std::string_view appName, std::shared_ptr<void> platformData);
		virtual ~IPlatform();

		inline std::string_view GetAppName() const noexcept { return m_AppName; }
		inline const PlatformConfig& GetPlatformConfig() const noexcept { return m_ConfigManager->GetPlatformConfig(); };
		PlatformFactory& GetFactory() noexcept;

		protected:
			std::string_view m_AppName;
			std::shared_ptr<void> m_PlatformData;
			std::shared_ptr<Path> m_Path;
			std::shared_ptr<ConfigManager> m_ConfigManager;

		private:
			void Initialize();
			virtual void InitializeImpl() = 0;

			std::unique_ptr<PlatformFactory> m_Factory;
	};
}