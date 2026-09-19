#include "ScriptFactory.h"

namespace zzz::core
{
	std::shared_ptr<Script> ScriptFactory::CreateScript(std::string_view name, GameObject* owner) const
	{
		return m_Storage.CreateScript(name, owner);
	}

	std::shared_ptr<GameScript> ScriptFactory::CreateGameScript(std::string_view name) const
	{
		return m_Storage.CreateGameScript(name);
	}

	std::shared_ptr<SceneScript> ScriptFactory::CreateSceneScript(std::string_view name) const
	{
		return m_Storage.CreateSceneScript(name);
	}

	std::shared_ptr<ViewScript> ScriptFactory::CreateViewScript(std::string_view name) const
	{
		return m_Storage.CreateViewScript(name);
	}

	std::shared_ptr<Script> ScriptFactory::CreateScript(const zzz::core::Guid& guid, GameObject* owner) const
	{
		return m_Storage.CreateScript(guid, owner);
	}

	std::shared_ptr<GameScript> ScriptFactory::CreateGameScript(const zzz::core::Guid& guid) const
	{
		return m_Storage.CreateGameScript(guid);
	}

	std::shared_ptr<SceneScript> ScriptFactory::CreateSceneScript(const zzz::core::Guid& guid) const
	{
		return m_Storage.CreateSceneScript(guid);
	}

	std::shared_ptr<ViewScript> ScriptFactory::CreateViewScript(const zzz::core::Guid& guid) const
	{
		return m_Storage.CreateViewScript(guid);
	}

	std::vector<std::string> ScriptFactory::GetAllGameScriptNames() const
	{
		return m_Storage.GetAllGameScriptNames();
	}

#if Z_EDITOR
	void ScriptFactory::RegisterInstance(Script* instance)       { m_Storage.RegisterInstance(instance); }
	void ScriptFactory::UnregisterInstance(Script* instance)     { m_Storage.UnregisterInstance(instance); }
	std::span<Script* const> ScriptFactory::GetActiveInstances() const { return m_Storage.GetActiveInstances(); }

	void ScriptFactory::RegisterInstance(GameScript* instance)   { m_Storage.RegisterInstance(instance); }
	void ScriptFactory::UnregisterInstance(GameScript* instance) { m_Storage.UnregisterInstance(instance); }
	std::span<GameScript* const> ScriptFactory::GetActiveGameScripts() const { return m_Storage.GetActiveGameScripts(); }

	void ScriptFactory::RegisterInstance(SceneScript* instance)  { m_Storage.RegisterInstance(instance); }
	void ScriptFactory::UnregisterInstance(SceneScript* instance){ m_Storage.UnregisterInstance(instance); }
	std::span<SceneScript* const> ScriptFactory::GetActiveSceneScripts() const { return m_Storage.GetActiveSceneScripts(); }

	void ScriptFactory::RegisterInstance(ViewScript* instance)   { m_Storage.RegisterInstance(instance); }
	void ScriptFactory::UnregisterInstance(ViewScript* instance) { m_Storage.UnregisterInstance(instance); }
	std::span<ViewScript* const> ScriptFactory::GetActiveViewScripts() const { return m_Storage.GetActiveViewScripts(); }
#endif
}
