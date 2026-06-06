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
		ConfigManager(std::shared_ptr<Path> path, std::string_view configPath);

		[[nodiscard]] std::expected<void, std::string> SaveConfig();

		inline const EngineConfig& GetEngineConfig() const noexcept { return *m_EngineConfig; }
		inline const PlatformConfig& GetPlatformConfig() const noexcept { return m_EngineConfig->GetPlatformConfig(); }

	private:
		void Initialize(std::string_view configPath);
		std::expected<std::filesystem::path, std::string> GetSettingsDirectory();
		std::expected<void, std::string> LoadConfig(std::filesystem::path path);

		std::shared_ptr<Path> m_Path;
		std::filesystem::path m_ConfigPath;
		std::shared_ptr<EngineConfig> m_EngineConfig;
		Serializer m_Serializer;

		bool m_IsDirty;
	};
}

