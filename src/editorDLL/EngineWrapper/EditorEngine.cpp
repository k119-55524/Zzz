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

	zzz::engine::View* EditorEngine::AddView(void* hwnd)
	{
		if (engineState.load() != eInitState::Running)
		{
			DOutError("EditorEngine is not running. Cannot AddView.");
			return nullptr;
		}

		return m_ViewManager->CreateView(hwnd);
	}

	void EditorEngine::RemoveView(void* view)
	{
		if (engineState.load() == eInitState::Running && view)
		{
			m_ViewManager->DestroyView(static_cast<zzz::engine::View*>(view));
		}
	}
}
