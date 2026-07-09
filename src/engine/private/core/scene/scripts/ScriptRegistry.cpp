
#include "GameScript.h"
#include "SceneScript.h"
#include "Script.h"
#include "ScriptRegistry.h"

namespace zzz::script
{
	std::unordered_map<std::string, std::function<std::shared_ptr<Script>(GameObject*)>> ScriptRegistry::s_ScriptFactories;
	std::unordered_map<std::string, std::function<std::shared_ptr<GameScript>()>> ScriptRegistry::s_GameScriptFactories;
	std::unordered_map<std::string, std::function<std::shared_ptr<SceneScript>()>> ScriptRegistry::s_SceneScriptFactories;

#if Z_EDITOR
	std::vector<Script*> ScriptRegistry::s_ActiveInstances;
#endif

	std::shared_ptr<Script> ScriptRegistry::CreateScript(std::string_view name, GameObject* owner)
	{
		auto it = s_ScriptFactories.find(std::string(name));
		if (it != s_ScriptFactories.end())
			return it->second(owner);

		return nullptr;
	}

	std::shared_ptr<GameScript> ScriptRegistry::CreateGameScript(std::string_view name)
	{
		auto it = s_GameScriptFactories.find(std::string(name));
		if (it != s_GameScriptFactories.end())
			return it->second();

		return nullptr;
	}

	std::shared_ptr<SceneScript> ScriptRegistry::CreateSceneScript(std::string_view name)
	{
		auto it = s_SceneScriptFactories.find(std::string(name));
		if (it != s_SceneScriptFactories.end())
			return it->second();

		return nullptr;
	}

	std::vector<std::string> ScriptRegistry::GetAllGameScriptNames()
	{
		std::vector<std::string> names;
		names.reserve(s_GameScriptFactories.size());

		for (const auto& [name, factory] : s_GameScriptFactories)
			names.push_back(name);

		return names;
	}

	void ScriptRegistry::Clear()
	{
		s_ScriptFactories.clear();
		s_GameScriptFactories.clear();
		s_SceneScriptFactories.clear();
#if Z_EDITOR
		s_ActiveInstances.clear();
#endif
	}

#if Z_EDITOR
	void ScriptRegistry::RegisterInstance(Script* instance)
	{
		if (instance)
			s_ActiveInstances.push_back(instance);
	}

	void ScriptRegistry::UnregisterInstance(Script* instance)
	{
		if (instance)
			std::erase(s_ActiveInstances, instance);
	}

	const std::vector<Script*>& ScriptRegistry::GetActiveInstances()
	{
		return s_ActiveInstances;
	}
#endif
}
