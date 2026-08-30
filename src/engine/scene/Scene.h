#pragma once

#include "engine/EngineIncludes.h"

namespace zzz::engine
{
	class ISurfView;
}

namespace zzz::engine
{
	using namespace zzz::core;

	class Scene final : public std::enable_shared_from_this<Scene>
	{
		Z_NO_COPY_MOVE(Scene);

	public:
		Scene() = delete;
		Scene(Guid guid, std::string name, const std::vector<Guid>& sceneScriptGuids, const ScriptFactory& scriptFactory);
		~Scene();

		[[nodiscard]] const Guid& GetGuid() const noexcept { return m_Guid; }
		[[nodiscard]] const std::string& GetName() const noexcept { return m_Name; }

		void Update(const Time& time);
		void PrepareFrame(ISurfView* surfView);
		void InvokeStart();
		void InvokeDestroy();

	private:
		Guid m_Guid;
		std::string m_Name;
		SceneEventBus m_EventBus;
		std::vector<std::shared_ptr<SceneScript>> m_Scripts;
	};
}
