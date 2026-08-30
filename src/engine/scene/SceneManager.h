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

	/**
	 * @brief Единственный владелец активных сцен движка (см. Scene - View хранит только weak_ptr на неё).
	 * @details Загружает Scene из package.dat через PackageManager с дедупликацией по Guid (LoadScene),
	 * централизованно обновляет все активные сцены (Update - вызывается СТРОГО 1 раз за кадр из
	 * Engine::OnUpdateSystem, до ViewManager::Update()) и безопасно выгружает их через отложенную очередь
	 * (UnloadScene ставит в очередь, ProcessPendingUnloads разбирает её в начале следующего Update() -
	 * это исключает мутацию m_ActiveScenes прямо во время итерации по ней в Update()).
	 */
	class SceneManager final
	{
		Z_NO_COPY_MOVE(SceneManager);

	public:
		SceneManager() = delete;
		SceneManager(std::shared_ptr<PackageManager> packageManager, std::shared_ptr<ScriptFactory> scriptFactory);
		~SceneManager() = default;

		/// @brief Возвращает уже загруженную сцену (дедупликация по Guid) либо загружает и запускает новую (InvokeStart()).
		[[nodiscard]] std::expected<std::shared_ptr<Scene>, std::string> LoadScene(const Guid& sceneGuid);
		/// @brief То же самое, но по имени сцены (имя -> Guid через PackageManager, затем LoadScene).
		[[nodiscard]] std::expected<std::shared_ptr<Scene>, std::string> LoadSceneByName(std::string_view sceneName);

		/**
		 * @brief Ставит сцену в очередь на выгрузку. Реальная выгрузка (InvokeDestroy() + удаление из
		 * m_ActiveScenes) происходит безопасно в начале СЛЕДУЮЩЕГО Update() (см. ProcessPendingUnloads),
		 * а не немедленно - UnloadScene() может быть вызван скриптом прямо во время текущей итерации
		 * Update() по активным сценам, и немедленное erase() было бы неопределённым поведением.
		 */
		void UnloadScene(const Guid& sceneGuid);

		/// @brief Централизованно обновляет все активные сцены. Вызывается СТРОГО 1 раз за кадр из Engine::OnUpdateSystem().
		void Update(const Time& time);

	private:
		void ProcessPendingUnloads();

		std::shared_ptr<PackageManager> m_PackageManager;
		std::shared_ptr<ScriptFactory> m_ScriptFactory;

		std::unordered_map<Guid, std::shared_ptr<Scene>> m_ActiveScenes;
		std::vector<Guid> m_PendingUnloads;
	};
}
