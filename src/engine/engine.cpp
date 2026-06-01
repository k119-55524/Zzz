#include "pch.h"

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
		initState{ eInitState::InitNot}
	{
		ensure(m_AppName.empty() == false, "Application name must not be empty.");
	}

	Engine::~Engine()
	{
		Shutdown();
	}

	void Engine::Shutdown()
	{
		try
		{
			if (initState != eInitState::InitNot)
			{
				auto res = m_ConfigManager->Serialize();
				if (!res)
					DOutFatal("Failed to serialize config: {}.", res.error());
			}

			m_ConfigManager = nullptr;
			m_Path = nullptr;
		}
		catch (const std::exception& e)
		{
			DOutException("Exception during shutdown: {}.", e.what());
		}
		catch (...)
		{
			DOutException("Unknown exception during shutdown.");
		}

		DOut("Engine shutdown completed.");
		initState = eInitState::InitNot;
	}

	std::expected<void, std::string> Engine::Initialize(std::string_view configPath)
	{
		std::lock_guard lock(stateMutex);

		if (initState != eInitState::InitNot)
			UNEXPECTED("Engine is already initialized or running.");

		try
		{
			DOut("Engine initialized: START.");

			m_Path = zzz::safe_make_shared<Path>(m_AppName, m_PlatformData);
			m_ConfigManager = zzz::safe_make_shared<ConfigManager>(m_Path);
			auto res = m_ConfigManager->Initialize(configPath);
			if (!res)
				UNEXPECTED("Failed to initialize ConfigManager: {}.", res.error());



			DOut("Engine initialized: END.");
			initState = eInitState::InitOK;

			return {};
		}
		catch (const std::exception& e)
		{
			UNEXPECTED("Exception initialize: {}.", e.what());
		}
		catch (...)
		{
			UNEXPECTED("Unknown exception occurred.");
		}
	}

	[[nodiscard]] std::expected<void, std::string> Engine::Run()
	{
		std::lock_guard lock(stateMutex);

		if (initState != eInitState::InitOK)
			UNEXPECTED("Engine is not initialized. Call Initialize() before Run().");

		if (initState == eInitState::Running)
			UNEXPECTED("Engine is already running.");

		initState = eInitState::Running;



		return {};
	}
}
