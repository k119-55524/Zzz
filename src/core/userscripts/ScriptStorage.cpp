#include "ScriptStorage.h"
#include "Script.h"
#include "ViewScript.h"
#include "GameScript.h"
#include "SceneScript.h"

namespace zzz::core
{
	std::unordered_map<std::string, ScriptStorage::ScriptFactoryFunc> ScriptStorage::s_ScriptFactories;
	std::unordered_map<std::string, ScriptStorage::GameScriptFactoryFunc> ScriptStorage::s_GameScriptFactories;
	std::unordered_map<std::string, ScriptStorage::SceneScriptFactoryFunc> ScriptStorage::s_SceneScriptFactories;
	std::unordered_map<std::string, ScriptStorage::ViewScriptFactoryFunc> ScriptStorage::s_ViewScriptFactories;

	std::unordered_map<zzz::core::Guid, ScriptStorage::ScriptFactoryFunc> ScriptStorage::s_ScriptGuidFactories;
	std::unordered_map<zzz::core::Guid, ScriptStorage::GameScriptFactoryFunc> ScriptStorage::s_GameScriptGuidFactories;
	std::unordered_map<zzz::core::Guid, ScriptStorage::SceneScriptFactoryFunc> ScriptStorage::s_SceneScriptGuidFactories;
	std::unordered_map<zzz::core::Guid, ScriptStorage::ViewScriptFactoryFunc> ScriptStorage::s_ViewScriptGuidFactories;

#if Z_EDITOR
	std::vector<Script*> ScriptStorage::s_ActiveInstances;
	std::vector<GameScript*> ScriptStorage::s_ActiveGameScripts;
	std::vector<SceneScript*> ScriptStorage::s_ActiveSceneScripts;
	std::vector<ViewScript*> ScriptStorage::s_ActiveViewScripts;
#endif

	namespace
	{
		template<typename MapType, typename KeyType, typename... Args>
		auto FindAndCreate(const MapType& map, const KeyType& key, Args&&... args)
		{
			auto it = map.find(key);
			if (it != map.end())
				return it->second(std::forward<Args>(args)...);

			using ReturnType = decltype(it->second(std::forward<Args>(args)...));
			return ReturnType{ nullptr };
		}

#if Z_EDITOR
		template<typename T>
		void RegisterInstanceImpl(T* instance, std::vector<T*>& vec)
		{
			if (instance)
				vec.push_back(instance);
		}

		template<typename T>
		void UnregisterInstanceImpl(T* instance, std::vector<T*>& vec)
		{
			if (instance)
				std::erase(vec, instance);
		}
#endif
	}

	void ScriptStorage::RegisterScriptFactory(const std::string& name, const zzz::core::Guid& guid, ScriptFactoryFunc factory)
	{
		s_ScriptFactories[name] = factory;
		if (!guid.IsEmpty())
			s_ScriptGuidFactories[guid] = factory;
	}

	void ScriptStorage::RegisterGameScriptFactory(const std::string& name, const zzz::core::Guid& guid, GameScriptFactoryFunc factory)
	{
		s_GameScriptFactories[name] = factory;
		if (!guid.IsEmpty())
			s_GameScriptGuidFactories[guid] = factory;
	}

	void ScriptStorage::RegisterSceneScriptFactory(const std::string& name, const zzz::core::Guid& guid, SceneScriptFactoryFunc factory)
	{
		s_SceneScriptFactories[name] = factory;
		if (!guid.IsEmpty())
			s_SceneScriptGuidFactories[guid] = factory;
	}

	void ScriptStorage::RegisterViewScriptFactory(const std::string& name, const zzz::core::Guid& guid, ViewScriptFactoryFunc factory)
	{
		s_ViewScriptFactories[name] = factory;
		if (!guid.IsEmpty())
			s_ViewScriptGuidFactories[guid] = factory;
	}

	std::shared_ptr<Script> ScriptStorage::CreateScript(std::string_view name, GameObject* owner)
	{
		return FindAndCreate(s_ScriptFactories, std::string(name), owner);
	}

	std::shared_ptr<GameScript> ScriptStorage::CreateGameScript(std::string_view name)
	{
		return FindAndCreate(s_GameScriptFactories, std::string(name));
	}

	std::shared_ptr<SceneScript> ScriptStorage::CreateSceneScript(std::string_view name)
	{
		return FindAndCreate(s_SceneScriptFactories, std::string(name));
	}

	std::shared_ptr<ViewScript> ScriptStorage::CreateViewScript(std::string_view name)
	{
		return FindAndCreate(s_ViewScriptFactories, std::string(name));
	}

	std::shared_ptr<Script> ScriptStorage::CreateScript(const zzz::core::Guid& guid, GameObject* owner)
	{
		return FindAndCreate(s_ScriptGuidFactories, guid, owner);
	}

	std::shared_ptr<GameScript> ScriptStorage::CreateGameScript(const zzz::core::Guid& guid)
	{
		return FindAndCreate(s_GameScriptGuidFactories, guid);
	}

	std::shared_ptr<SceneScript> ScriptStorage::CreateSceneScript(const zzz::core::Guid& guid)
	{
		return FindAndCreate(s_SceneScriptGuidFactories, guid);
	}

	std::shared_ptr<ViewScript> ScriptStorage::CreateViewScript(const zzz::core::Guid& guid)
	{
		return FindAndCreate(s_ViewScriptGuidFactories, guid);
	}

	std::vector<std::string> ScriptStorage::GetAllGameScriptNames()
	{
		std::vector<std::string> names;
		names.reserve(s_GameScriptFactories.size());

		for (const auto& [name, factory] : s_GameScriptFactories)
			names.push_back(name);

		return names;
	}

	void ScriptStorage::Clear()
	{
		s_ScriptFactories.clear();
		s_GameScriptFactories.clear();
		s_SceneScriptFactories.clear();
		s_ViewScriptFactories.clear();

		s_ScriptGuidFactories.clear();
		s_GameScriptGuidFactories.clear();
		s_SceneScriptGuidFactories.clear();
		s_ViewScriptGuidFactories.clear();
#if Z_EDITOR
		s_ActiveInstances.clear();
		s_ActiveGameScripts.clear();
		s_ActiveSceneScripts.clear();
		s_ActiveViewScripts.clear();
#endif
	}

#if Z_EDITOR
	void ScriptStorage::RegisterInstance(Script* instance)       { RegisterInstanceImpl(instance, s_ActiveInstances); }
	void ScriptStorage::UnregisterInstance(Script* instance)     { UnregisterInstanceImpl(instance, s_ActiveInstances); }
	const std::vector<Script*>& ScriptStorage::GetActiveInstances() { return s_ActiveInstances; }

	void ScriptStorage::RegisterInstance(GameScript* instance)   { RegisterInstanceImpl(instance, s_ActiveGameScripts); }
	void ScriptStorage::UnregisterInstance(GameScript* instance) { UnregisterInstanceImpl(instance, s_ActiveGameScripts); }
	const std::vector<GameScript*>& ScriptStorage::GetActiveGameScripts() { return s_ActiveGameScripts; }

	void ScriptStorage::RegisterInstance(SceneScript* instance)  { RegisterInstanceImpl(instance, s_ActiveSceneScripts); }
	void ScriptStorage::UnregisterInstance(SceneScript* instance){ UnregisterInstanceImpl(instance, s_ActiveSceneScripts); }
	const std::vector<SceneScript*>& ScriptStorage::GetActiveSceneScripts() { return s_ActiveSceneScripts; }

	void ScriptStorage::RegisterInstance(ViewScript* instance)   { RegisterInstanceImpl(instance, s_ActiveViewScripts); }
	void ScriptStorage::UnregisterInstance(ViewScript* instance) { UnregisterInstanceImpl(instance, s_ActiveViewScripts); }
	const std::vector<ViewScript*>& ScriptStorage::GetActiveViewScripts() { return s_ActiveViewScripts; }
#endif
}
