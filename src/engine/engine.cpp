
#include "engine.h"
#include "private/core/IO/Path.h"
#include "private/core/Config/ConfigManager.h"

using namespace zzz;
using namespace zzz::io;

namespace zzz::engine
{
	Engine::Engine(std::string_view appName, std::shared_ptr<void> platformData) :
		m_AppName{ appName },
		m_PlatformData{ platformData },
		initState{zzz::InitNot}
	{
		ensure(m_AppName.empty() == false, "Application name must not be empty.");
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

			m_Path = zzz::safe_make_shared<Path>(m_AppName, m_PlatformData);
			m_ConfigManager = zzz::safe_make_shared<ConfigManager>(m_Path);
			auto res = m_ConfigManager->Initialize(configPath);
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
