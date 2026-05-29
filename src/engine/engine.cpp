
#include "engine.h"
#include "private/core/Config/ConfigManager.h"

namespace zzz::engine
{
	Engine::Engine() :
		initState{zzz::InitNot}
	{
	}

	std::expected<void, std::string> Engine::Initialize(std::string_view configPath)
	{
		std::lock_guard lock(initMutex);

		if (initState != InitNot)
		{
			DOut("Engine is already initialized or in the process of initialization.");
			return std::unexpected("Engine is already initialized or in the process of initialization.");
		}

		try
		{
			DOutLite("Engine initialized: START.");

			configManager = zzz::safe_make_shared<ConfigManager>();
			auto res = configManager->Initialize(configPath);
			if (!res)
			{
				DOut("Failed to initialize ConfigManager: {}.", res.error());
				return std::unexpected(res.error());
			}

			DOutLite("Engine initialized: END.");
			initState = zzz::InitOK;

			return {};
		}
		catch (const std::exception& e)
		{
			return std::unexpected(std::format("Exception initialize: {}.", e.what()));
		}
		catch (...)
		{
			return std::unexpected("Unknown exception occurred.");
		}
	}
}
