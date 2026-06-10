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
		ConfigManager(const Path& path);

		[[nodiscard]] std::expected<void, std::string> SaveConfig();

		inline const PlatformConfig& GetPlatformConfig() const noexcept { return m_EngineConfig->GetPlatformConfig(); }

	private:
		void Initialize();
		std::expected<std::filesystem::path, std::string> GetSettingsDirectory();
		std::expected<void, std::string> LoadConfig(std::filesystem::path path);

		std::filesystem::path m_ConfigPath;
		Path m_Path;
		std::shared_ptr<EngineConfig> m_EngineConfig;

		Serializer m_Serializer;
		bool m_IsDirty;
	};
}

