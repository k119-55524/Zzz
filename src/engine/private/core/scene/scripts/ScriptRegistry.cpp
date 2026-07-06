
#include "Game.h"
#include "../Scene.h"
#include "Script.h"
#include "ScriptRegistry.h"

namespace zzz::script
{
	std::unordered_map<std::string, std::function<std::shared_ptr<Script>(GameObject*)>> ScriptRegistry::s_ScriptFactories;
	std::unordered_map<std::string, std::function<std::shared_ptr<Game>()>> ScriptRegistry::s_GameFactories;
	std::unordered_map<std::string, std::function<std::shared_ptr<Scene>()>> ScriptRegistry::s_SceneFactories;

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

	std::shared_ptr<Game> ScriptRegistry::CreateGame(std::string_view name)
	{
		auto it = s_GameFactories.find(std::string(name));
		if (it != s_GameFactories.end())
			return it->second();

		return nullptr;
	}

	std::shared_ptr<Scene> ScriptRegistry::CreateScene(std::string_view name)
	{
		auto it = s_SceneFactories.find(std::string(name));
		if (it != s_SceneFactories.end())
			return it->second();

		return nullptr;
	}

	std::vector<std::string> ScriptRegistry::GetAllGameNames()
	{
		std::vector<std::string> names;
		names.reserve(s_GameFactories.size());

		for (const auto& [name, factory] : s_GameFactories)
			names.push_back(name);

		return names;
	}

	void ScriptRegistry::Clear()
	{
		s_ScriptFactories.clear();
		s_GameFactories.clear();
		s_SceneFactories.clear();
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
