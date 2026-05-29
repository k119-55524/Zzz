#pragma once

#include "header.h"

namespace zzz::engine
{
	class ConfigManager final
	{
	public:
		ConfigManager();

		[[nodiscard]] std::expected<void, std::string> Initialize(std::string_view configPath);

	private:
		[[nodiscard]] std::expected<std::filesystem::path, std::string> GetSettingsDirectory();
	};
}
