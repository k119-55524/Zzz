#pragma once

#include <memory>
#include <vector>
#include <string>
#include <functional>
#include <string_view>
#include <unordered_map>

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
	class Z_CORE_API ScriptStorage
	{
	public:
		using ScriptFactoryFunc = std::function<std::shared_ptr<Script>(GameObject*)>;
		using GameScriptFactoryFunc = std::function<std::shared_ptr<GameScript>()>;
		using SceneScriptFactoryFunc = std::function<std::shared_ptr<SceneScript>()>;
		using ViewScriptFactoryFunc = std::function<std::shared_ptr<ViewScript>()>;

		// Регистрация фабрик
		static void RegisterScriptFactory(const std::string& name, const zzz::core::Guid& guid, ScriptFactoryFunc factory);
		static void RegisterGameScriptFactory(const std::string& name, const zzz::core::Guid& guid, GameScriptFactoryFunc factory);
		static void RegisterSceneScriptFactory(const std::string& name, const zzz::core::Guid& guid, SceneScriptFactoryFunc factory);
		static void RegisterViewScriptFactory(const std::string& name, const zzz::core::Guid& guid, ViewScriptFactoryFunc factory);

		// Создание инстансов по имени
		static std::shared_ptr<Script> CreateScript(std::string_view name, GameObject* owner);
		static std::shared_ptr<GameScript> CreateGameScript(std::string_view name);
		static std::shared_ptr<SceneScript> CreateSceneScript(std::string_view name);
		static std::shared_ptr<ViewScript> CreateViewScript(std::string_view name);

		// Создание инстансов по GUID
		static std::shared_ptr<Script> CreateScript(const zzz::core::Guid& guid, GameObject* owner);
		static std::shared_ptr<GameScript> CreateGameScript(const zzz::core::Guid& guid);
		static std::shared_ptr<SceneScript> CreateSceneScript(const zzz::core::Guid& guid);
		static std::shared_ptr<ViewScript> CreateViewScript(const zzz::core::Guid& guid);

		// Получение списков
		static std::vector<std::string> GetAllGameScriptNames();

		// Очистка
		static void Clear();

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

	private:
		static std::unordered_map<std::string, ScriptFactoryFunc> s_ScriptFactories;
		static std::unordered_map<std::string, GameScriptFactoryFunc> s_GameScriptFactories;
		static std::unordered_map<std::string, SceneScriptFactoryFunc> s_SceneScriptFactories;
		static std::unordered_map<std::string, ViewScriptFactoryFunc> s_ViewScriptFactories;

		static std::unordered_map<zzz::core::Guid, ScriptFactoryFunc> s_ScriptGuidFactories;
		static std::unordered_map<zzz::core::Guid, GameScriptFactoryFunc> s_GameScriptGuidFactories;
		static std::unordered_map<zzz::core::Guid, SceneScriptFactoryFunc> s_SceneScriptGuidFactories;
		static std::unordered_map<zzz::core::Guid, ViewScriptFactoryFunc> s_ViewScriptGuidFactories;

#if Z_EDITOR
		static std::vector<Script*> s_ActiveInstances;
		static std::vector<GameScript*> s_ActiveGameScripts;
		static std::vector<SceneScript*> s_ActiveSceneScripts;
		static std::vector<ViewScript*> s_ActiveViewScripts;
#endif
	};
#pragma warning(pop)
}
