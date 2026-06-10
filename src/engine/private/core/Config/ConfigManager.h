#pragma once

#include "../io/Path.h"
#include "EngineConfig.h"

using namespace zzz::io;

namespace zzz::engine
{
	enum class eInitConfigState
	{
		InitOK,
		InitDefault,
	};

	class ConfigManager final
	{
	public:
		ConfigManager() = delete;
		ConfigManager(const Path& path, std::shared_ptr<IConfig> platformConfig);

		[[nodiscard]] std::expected<void, std::string> SaveConfig();

		inline const std::shared_ptr<EngineConfig> GetEngineConfig() const noexcept { return m_EngineConfig; }
		inline const IConfig& GetPlatformConfig() const noexcept { return m_EngineConfig->GetPlatformConfig(); }

	private:
		void Initialize();
		std::expected<std::filesystem::path, std::string> GetSettingsDirectory();
		std::expected<void, std::string> LoadConfig(std::filesystem::path path);

		std::shared_ptr<IConfig> m_PlatformConfig;
		Path m_Path;
		std::filesystem::path m_ConfigPath;
		std::shared_ptr<EngineConfig> m_EngineConfig;
		Serializer m_Serializer;

		bool m_IsDirty;
	};
}

