#pragma once

#include <mutex>
#include "header.h"
#include "headers/enums.h"

namespace zzz::io
{
	class Path;
}

namespace zzz::engine
{
	class ConfigManager;
}

using namespace zzz;

namespace zzz::engine
{
	class Engine final
	{
	public:
		Engine() = delete;
		Engine(std::string_view appName, std::shared_ptr<void> platformData = nullptr);
		~Engine();

		[[nodiscard]] std::expected<void, std::string> Initialize(std::string_view configPath = {});
		[[nodiscard]] std::expected<void, std::string> Run();

	private:
		void Shutdown();

		std::string_view m_AppName;
		std::shared_ptr<void> m_PlatformData;

		std::mutex stateMutex;
		eInitState initState;

		std::shared_ptr<io::Path> m_Path;
		std::shared_ptr<ConfigManager> m_ConfigManager;
	};
}
