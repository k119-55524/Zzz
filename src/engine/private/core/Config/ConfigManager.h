#pragma once

#include "../IO/Path.h"
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
		ConfigManager(std::shared_ptr<Path> path);

		[[nodiscard]] std::expected<eInitConfigState, std::string> Initialize(std::string_view configPath);
		[[nodiscard]] std::expected<void, std::string> SaveConfig();

		inline const PlatformConfig& GetPlatformConfig() const noexcept { return m_EngineConfig->GetPlatformConfig(); }

	private:
		std::expected<std::filesystem::path, std::string> GetSettingsDirectory();
		std::expected<void, std::string> LoadConfig(std::filesystem::path path);

		std::shared_ptr<Path> m_Path;
		std::filesystem::path m_ConfigPath;
		std::shared_ptr<EngineConfig> m_EngineConfig;
		Serializer m_Serializer;

		bool m_IsDirty;
	};
}
