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

	private:
		std::expected<std::filesystem::path, std::string> GetSettingsDirectory();

		std::shared_ptr<Path> m_Path;
		std::shared_ptr<EngineConfig> engineConfig;
	};
}
