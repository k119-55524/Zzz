#pragma once
#include <span>

#include <memory>
#include <vector>
#include <string>
#include <string_view>

#include "core/utils/Guid.h"
#include "core/utils/Export.h"
#include "core/utils/Macroses.h"
#include "ScriptStorage.h"

namespace zzz::engine
{
	class GameObject;
}

namespace zzz
{
	using GameObject = ::zzz::engine::GameObject;
}

namespace zzz::core
{
	class Script;
	class GameScript;
	class SceneScript;
	class ViewScript;

#pragma warning(push)
#pragma warning(disable: 4251)
	class Z_CORE_API ScriptFactory final
	{
		Z_NO_COPY_MOVE(ScriptFactory);

	public:
		ScriptFactory() = delete;
		explicit ScriptFactory(ScriptStorage& storage) : m_Storage(storage) {}

		// Создание объектов по имени
		[[nodiscard]] std::shared_ptr<Script> CreateScript(std::string_view name, GameObject* owner) const;
		[[nodiscard]] std::shared_ptr<GameScript> CreateGameScript(std::string_view name) const;
		[[nodiscard]] std::shared_ptr<SceneScript> CreateSceneScript(std::string_view name) const;
		[[nodiscard]] std::shared_ptr<ViewScript> CreateViewScript(std::string_view name) const;

		// Создание объектов по GUID
		[[nodiscard]] std::shared_ptr<Script> CreateScript(const zzz::core::Guid& guid, GameObject* owner) const;
		[[nodiscard]] std::shared_ptr<GameScript> CreateGameScript(const zzz::core::Guid& guid) const;
		[[nodiscard]] std::shared_ptr<SceneScript> CreateSceneScript(const zzz::core::Guid& guid) const;
		[[nodiscard]] std::shared_ptr<ViewScript> CreateViewScript(const zzz::core::Guid& guid) const;

		// Получение списка всех Game-скриптов
		[[nodiscard]] std::vector<std::string> GetAllGameScriptNames() const;

#if Z_EDITOR
		void RegisterInstance(Script* instance);
		void UnregisterInstance(Script* instance);
		[[nodiscard]] std::span<Script* const> GetActiveInstances() const;

		void RegisterInstance(GameScript* instance);
		void UnregisterInstance(GameScript* instance);
		[[nodiscard]] std::span<GameScript* const> GetActiveGameScripts() const;

		void RegisterInstance(SceneScript* instance);
		void UnregisterInstance(SceneScript* instance);
		[[nodiscard]] std::span<SceneScript* const> GetActiveSceneScripts() const;

		void RegisterInstance(ViewScript* instance);
		void UnregisterInstance(ViewScript* instance);
		[[nodiscard]] std::span<ViewScript* const> GetActiveViewScripts() const;
#endif

	private:
		ScriptStorage& m_Storage;
	};
#pragma warning(pop)
}
