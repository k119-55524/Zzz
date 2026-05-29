#pragma once

#include "header.h"

namespace zzz::engine
{
	class ConfigManager;

	class Engine final
	{
	public:
		Engine();

		[[nodiscard]] std::expected<void, std::string> Initialize(std::string_view configPath = {});

	private:
		std::mutex initMutex;
		eInitState initState;

		std::shared_ptr<ConfigManager> configManager;
	};
}
