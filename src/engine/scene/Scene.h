#pragma once

#include "engine/EngineIncludes.h"

namespace zzz::engine
{
	class ISurfView;
}

namespace zzz::engine
{
	using namespace zzz::core;

	/**
	 * @brief Загруженная игровая сцена: набор SceneScript-компонентов и собственная шина событий сцены.
	 * @details Единственный владелец экземпляров Scene - SceneManager (см. SceneManager::m_ActiveScenes,
	 * дедупликация по Guid). View хранит только std::weak_ptr<Scene> и не управляет её жизненным циклом
	 * (см. View::PrepareFrame - lock() + PrepareFrame(surfView)).
	 *
	 * InvokeStart()/InvokeDestroy() вызываются ИСКЛЮЧИТЕЛЬНО из SceneManager (LoadScene / ProcessPendingUnloads) -
	 * деструктор Scene НЕ вызывает InvokeDestroy() самостоятельно, чтобы не сработать OnDestroy() дважды.
	 */
	class Scene final : public std::enable_shared_from_this<Scene>
	{
		Z_NO_COPY_MOVE(Scene);

	public:
		Scene() = delete;

		/**
		 * @brief Создаёт сцену и сразу инстанцирует и инициализирует все её SceneScript по guid'ам из SceneData.
		 * @param guid Guid сцены (из PackageEntry).
		 * @param name Имя сцены (из PackageEntry, для LoadSceneByName/логов).
		 * @param sceneScriptGuids Guid'ы скриптов сцены (SceneData::GetSceneScriptGuids()).
		 * @param scriptFactory Фабрика для инстанцирования SceneScript по guid.
		 */
		Scene(Guid guid, std::string name, const std::vector<Guid>& sceneScriptGuids, const ScriptFactory& scriptFactory);
		~Scene();

		[[nodiscard]] const Guid& GetGuid() const noexcept { return m_Guid; }
		[[nodiscard]] const std::string& GetName() const noexcept { return m_Name; }

		/// @brief Обновление логики сцены (SceneScript::OnUpdate через шину событий). СТРОГО 1 раз за кадр, из SceneManager::Update().
		void Update(const Time& time);

		/// @brief Заготовка сбора команд отрисовки для конкретного окна/камеры. Вызывается из View::PrepareFrame() (Поток 1).
		/// @details Реальное построение рендер-списков (меши/материалы) - предмет будущей задачи (ResourceManager), см. docs/ARCHITECTURE.md.
		void PrepareFrame(ISurfView* surfView);

		/// @brief Запускает OnStart() у всех скриптов сцены. Вызывается SceneManager::LoadScene() один раз, сразу после создания.
		void InvokeStart();
		/// @brief Запускает OnDestroy() у всех скриптов сцены. Вызывается SceneManager перед удалением сцены из m_ActiveScenes.
		void InvokeDestroy();

	private:
		Guid m_Guid;
		std::string m_Name;
		SceneEventBus m_EventBus;
		std::vector<std::shared_ptr<SceneScript>> m_Scripts;
	};
}
