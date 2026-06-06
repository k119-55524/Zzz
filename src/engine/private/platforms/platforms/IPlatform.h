#pragma once

#include "../../core/io/Path.h"
#include "../../core/config/ConfigManager.h"

using namespace zzz::io;

namespace zzz::engine
{
	class Engine;

	class IPlatform
	{
		friend class Engine;

	public:
		IPlatform() = delete;
		IPlatform(std::string_view appName, std::shared_ptr<void> platformData);
		virtual ~IPlatform();

		inline std::string_view GetAppName() const noexcept { return m_AppName; }
		// TODO: не правильная архитектура. Подумать как задавать размер окна
		inline const Size2D<zU32>& GetWinSize() const noexcept { return m_ConfigManager->GetEngineConfig()->GetWinSize(); }
		inline const PlatformConfig& GetPlatformConfig() const noexcept { return m_ConfigManager->GetPlatformConfig(); };

		protected:
			std::string_view m_AppName;
			std::shared_ptr<void> m_PlatformData;
			std::shared_ptr<Path> m_Path;
			std::shared_ptr<ConfigManager> m_ConfigManager;

		private:
			void Initialize(std::string_view configPath);
			virtual void InitializeImpl() = 0;
	};
}