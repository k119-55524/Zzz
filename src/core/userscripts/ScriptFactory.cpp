#include "ScriptFactory.h"
#include "ScriptStorage.h"

namespace zzz::core
{
	std::shared_ptr<Script> ScriptFactory::CreateScript(std::string_view name, GameObject* owner)
	{
		return ScriptStorage::CreateScript(name, owner);
	}

	std::shared_ptr<GameScript> ScriptFactory::CreateGameScript(std::string_view name)
	{
		return ScriptStorage::CreateGameScript(name);
	}

	std::shared_ptr<SceneScript> ScriptFactory::CreateSceneScript(std::string_view name)
	{
		return ScriptStorage::CreateSceneScript(name);
	}

	std::shared_ptr<ViewScript> ScriptFactory::CreateViewScript(std::string_view name)
	{
		return ScriptStorage::CreateViewScript(name);
	}

	std::shared_ptr<Script> ScriptFactory::CreateScript(const zzz::core::Guid& guid, GameObject* owner)
	{
		return ScriptStorage::CreateScript(guid, owner);
	}

	std::shared_ptr<GameScript> ScriptFactory::CreateGameScript(const zzz::core::Guid& guid)
	{
		return ScriptStorage::CreateGameScript(guid);
	}

	std::shared_ptr<SceneScript> ScriptFactory::CreateSceneScript(const zzz::core::Guid& guid)
	{
		return ScriptStorage::CreateSceneScript(guid);
	}

	std::shared_ptr<ViewScript> ScriptFactory::CreateViewScript(const zzz::core::Guid& guid)
	{
		return ScriptStorage::CreateViewScript(guid);
	}

	std::vector<std::string> ScriptFactory::GetAllGameScriptNames()
	{
		return ScriptStorage::GetAllGameScriptNames();
	}

#if Z_EDITOR
	void ScriptFactory::RegisterInstance(Script* instance)       { ScriptStorage::RegisterInstance(instance); }
	void ScriptFactory::UnregisterInstance(Script* instance)     { ScriptStorage::UnregisterInstance(instance); }
	const std::vector<Script*>& ScriptFactory::GetActiveInstances() { return ScriptStorage::GetActiveInstances(); }

	void ScriptFactory::RegisterInstance(GameScript* instance)   { ScriptStorage::RegisterInstance(instance); }
	void ScriptFactory::UnregisterInstance(GameScript* instance) { ScriptStorage::UnregisterInstance(instance); }
	const std::vector<GameScript*>& ScriptFactory::GetActiveGameScripts() { return ScriptStorage::GetActiveGameScripts(); }

	void ScriptFactory::RegisterInstance(SceneScript* instance)  { ScriptStorage::RegisterInstance(instance); }
	void ScriptFactory::UnregisterInstance(SceneScript* instance){ ScriptStorage::UnregisterInstance(instance); }
	const std::vector<SceneScript*>& ScriptFactory::GetActiveSceneScripts() { return ScriptStorage::GetActiveSceneScripts(); }

	void ScriptFactory::RegisterInstance(ViewScript* instance)   { ScriptStorage::RegisterInstance(instance); }
	void ScriptFactory::UnregisterInstance(ViewScript* instance) { ScriptStorage::UnregisterInstance(instance); }
	const std::vector<ViewScript*>& ScriptFactory::GetActiveViewScripts() { return ScriptStorage::GetActiveViewScripts(); }
#endif
}
