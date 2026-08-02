#pragma once

#include "UserSettings.h"
#include <engine/private/core/IO/Path.h>

using namespace zzz::io;

namespace zzz::engine
{
	class ConfigManager final
	{
	public:
		ConfigManager() = delete;
		ConfigManager(const Path& path);

		inline const PlatformConfig& GetPlatformConfig() const noexcept { return m_UserSettings->GetPlatformConfig(); }
		[[nodiscard]] std::expected<void, std::string> SaveConfig();

	private:
		void Initialize();
		std::expected<std::filesystem::path, std::string> GetSettingsDirectory();
		std::expected<void, std::string> LoadConfig(std::filesystem::path path);

		Path m_Path;
		std::filesystem::path m_ConfigPath;
		std::shared_ptr<UserSettings> m_UserSettings;

		Serializer m_Serializer;
		bool m_IsDirty;
	};
}
