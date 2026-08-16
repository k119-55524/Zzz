#pragma once

#include <memory>
#include <vector>
#include <string>
#include <string_view>

#include "core/utils/Guid.h"
#include "core/utils/Export.h"

namespace zzz
{
	class GameObject;
}

namespace zzz::core
{
	class Script;
	class GameScript;
	class SceneScript;
	class ViewScript;

#pragma warning(push)
#pragma warning(disable: 4251)
	class Z_CORE_API ScriptFactory
	{
	public:
		// Создание объектов по имени
		static std::shared_ptr<Script> CreateScript(std::string_view name, GameObject* owner);
		static std::shared_ptr<GameScript> CreateGameScript(std::string_view name);
		static std::shared_ptr<SceneScript> CreateSceneScript(std::string_view name);
		static std::shared_ptr<ViewScript> CreateViewScript(std::string_view name);

		// Создание объектов по GUID
		static std::shared_ptr<Script> CreateScript(const zzz::core::Guid& guid, GameObject* owner);
		static std::shared_ptr<GameScript> CreateGameScript(const zzz::core::Guid& guid);
		static std::shared_ptr<SceneScript> CreateSceneScript(const zzz::core::Guid& guid);
		static std::shared_ptr<ViewScript> CreateViewScript(const zzz::core::Guid& guid);

		// Получение списка всех Game-скриптов
		static std::vector<std::string> GetAllGameScriptNames();

#if Z_EDITOR
		static void RegisterInstance(Script* instance);
		static void UnregisterInstance(Script* instance);
		static const std::vector<Script*>& GetActiveInstances();

		static void RegisterInstance(GameScript* instance);
		static void UnregisterInstance(GameScript* instance);
		static const std::vector<GameScript*>& GetActiveGameScripts();

		static void RegisterInstance(SceneScript* instance);
		static void UnregisterInstance(SceneScript* instance);
		static const std::vector<SceneScript*>& GetActiveSceneScripts();

		static void RegisterInstance(ViewScript* instance);
		static void UnregisterInstance(ViewScript* instance);
		static const std::vector<ViewScript*>& GetActiveViewScripts();
#endif
	};
#pragma warning(pop)
}
