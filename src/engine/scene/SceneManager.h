#pragma once

#include "engine/EngineIncludes.h"

namespace zzz::engine
{
	class PackageManager;
	class Scene;
}

namespace zzz::engine
{
	using namespace zzz::core;

	class SceneManager final
	{
		Z_NO_COPY_MOVE(SceneManager);

	public:
		SceneManager() = delete;
		SceneManager(std::shared_ptr<PackageManager> packageManager, std::shared_ptr<ScriptFactory> scriptFactory);
		~SceneManager() = default;

		[[nodiscard]] std::expected<std::shared_ptr<Scene>, std::string> LoadScene(const Guid& sceneGuid);
		[[nodiscard]] std::expected<std::shared_ptr<Scene>, std::string> LoadSceneByName(std::string_view sceneName);

		void Update(const Time& time);

	private:
		std::shared_ptr<PackageManager> m_PackageManager;
		std::shared_ptr<ScriptFactory> m_ScriptFactory;

		std::unordered_map<Guid, std::shared_ptr<Scene>> m_ActiveScenes;
	};
}
