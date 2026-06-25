#include "EditorEngine.h"
#include <engine/headers/enums.h>
#include <engine/private/core/view/ViewManager.h>
#include <logger/logger.h>

using namespace zzz;
using namespace zzz::engine;
using namespace zzz::logger;

namespace zzz::editor
{
	[[nodiscard]] std::expected<void, std::string> EditorEngine::Run()
	{
		std::lock_guard lock(stateMutex);

		if (engineState.load() != eInitState::Initialized)
			return UNEXPECTED("Engine is not initialized.");

		engineState.store(eInitState::Running);

		try
		{
			m_ViewManager->CreateView();
		}
		catch (const std::exception& e)
		{
			Shutdown();
			return UNEXPECTED("Exception creating view in Editor Run: {}.", e.what());
		}
		catch (...)
		{
			Shutdown();
			return UNEXPECTED("Unknown exception creating view in Editor Run.");
		}

		return {};
	}

	void EditorEngine::Tick()
	{
		if (engineState.load() == eInitState::Running)
		{
			OnUpdateSystem();
		}
	}

	void EditorEngine::ClearEngine()
	{
		DOut("EditorEngine::ClearEngine called: project-specific resources cleared.");
	}
}
