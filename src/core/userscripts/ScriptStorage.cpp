#include "ScriptStorage.h"
#include "core/userscripts/base_script/Script.h"
#include "core/userscripts/base_script/ViewScript.h"
#include "core/userscripts/base_script/GameScript.h"
#include "core/userscripts/base_script/SceneScript.h"
#include "core/utils/ThrowWrappers.h"
#include "core/utils/Macroses.h"

namespace zzz::core
{
	namespace
	{
		template<typename MapType, typename KeyType, typename... Args>
		auto FindAndCreate(const MapType& map, const KeyType& key, std::string_view scriptType, Args&&... args)
		{
			auto it = map.find(key);
			if (it != map.end())
				return it->second(std::forward<Args>(args)...);

			if constexpr (std::is_same_v<KeyType, zzz::core::Guid>)
			{
				THROW_RUNTIME("Не удалось создать {} с GUID '{}': фабрика скрипта не зарегистрирована в ScriptStorage.", scriptType, key.ToString());
			}
			else
			{
				THROW_RUNTIME("Не удалось создать {} с именем '{}': фабрика скрипта не зарегистрирована в ScriptStorage.", scriptType, key);
			}
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
		m_ScriptFactories[name] = factory;
		if (!guid.IsEmpty())
			m_ScriptGuidFactories[guid] = factory;
	}

	void ScriptStorage::RegisterGameScriptFactory(const std::string& name, const zzz::core::Guid& guid, GameScriptFactoryFunc factory)
	{
		m_GameScriptFactories[name] = factory;
		if (!guid.IsEmpty())
			m_GameScriptGuidFactories[guid] = factory;
	}

	void ScriptStorage::RegisterSceneScriptFactory(const std::string& name, const zzz::core::Guid& guid, SceneScriptFactoryFunc factory)
	{
		m_SceneScriptFactories[name] = factory;
		if (!guid.IsEmpty())
			m_SceneScriptGuidFactories[guid] = factory;
	}

	void ScriptStorage::RegisterViewScriptFactory(const std::string& name, const zzz::core::Guid& guid, ViewScriptFactoryFunc factory)
	{
		m_ViewScriptFactories[name] = factory;
		if (!guid.IsEmpty())
			m_ViewScriptGuidFactories[guid] = factory;
	}

	std::shared_ptr<Script> ScriptStorage::CreateScript(std::string_view name, GameObject* owner) const
	{
		auto script = FindAndCreate(m_ScriptFactories, std::string(name), "Script", owner);
#if Z_EDITOR
		if (script)
			const_cast<ScriptStorage*>(this)->m_ActiveInstances.push_back(script.get());
#endif
		return script;
	}

	std::shared_ptr<GameScript> ScriptStorage::CreateGameScript(std::string_view name) const
	{
		auto script = FindAndCreate(m_GameScriptFactories, std::string(name), "GameScript");
#if Z_EDITOR
		if (script)
			const_cast<ScriptStorage*>(this)->m_ActiveGameScripts.push_back(script.get());
#endif
		return script;
	}

	std::shared_ptr<SceneScript> ScriptStorage::CreateSceneScript(std::string_view name) const
	{
		auto script = FindAndCreate(m_SceneScriptFactories, std::string(name), "SceneScript");
#if Z_EDITOR
		if (script)
			const_cast<ScriptStorage*>(this)->m_ActiveSceneScripts.push_back(script.get());
#endif
		return script;
	}

	std::shared_ptr<ViewScript> ScriptStorage::CreateViewScript(std::string_view name) const
	{
		auto script = FindAndCreate(m_ViewScriptFactories, std::string(name), "ViewScript");
#if Z_EDITOR
		if (script)
			const_cast<ScriptStorage*>(this)->m_ActiveViewScripts.push_back(script.get());
#endif
		return script;
	}

	std::shared_ptr<Script> ScriptStorage::CreateScript(const zzz::core::Guid& guid, GameObject* owner) const
	{
		auto script = FindAndCreate(m_ScriptGuidFactories, guid, "Script", owner);
#if Z_EDITOR
		if (script)
			const_cast<ScriptStorage*>(this)->m_ActiveInstances.push_back(script.get());
#endif
		return script;
	}

	std::shared_ptr<GameScript> ScriptStorage::CreateGameScript(const zzz::core::Guid& guid) const
	{
		auto script = FindAndCreate(m_GameScriptGuidFactories, guid, "GameScript");
#if Z_EDITOR
		if (script)
			const_cast<ScriptStorage*>(this)->m_ActiveGameScripts.push_back(script.get());
#endif
		return script;
	}

	std::shared_ptr<SceneScript> ScriptStorage::CreateSceneScript(const zzz::core::Guid& guid) const
	{
		auto script = FindAndCreate(m_SceneScriptGuidFactories, guid, "SceneScript");
#if Z_EDITOR
		if (script)
			const_cast<ScriptStorage*>(this)->m_ActiveSceneScripts.push_back(script.get());
#endif
		return script;
	}

	std::shared_ptr<ViewScript> ScriptStorage::CreateViewScript(const zzz::core::Guid& guid) const
	{
		auto script = FindAndCreate(m_ViewScriptGuidFactories, guid, "ViewScript");
#if Z_EDITOR
		if (script)
			const_cast<ScriptStorage*>(this)->m_ActiveViewScripts.push_back(script.get());
#endif
		return script;
	}

	std::vector<std::string> ScriptStorage::GetAllGameScriptNames() const
	{
		std::vector<std::string> names;
		names.reserve(m_GameScriptFactories.size());

		for (const auto& [name, factory] : m_GameScriptFactories)
			names.push_back(name);

		return names;
	}

	void ScriptStorage::Clear()
	{
		m_ScriptFactories.clear();
		m_GameScriptFactories.clear();
		m_SceneScriptFactories.clear();
		m_ViewScriptFactories.clear();

		m_ScriptGuidFactories.clear();
		m_GameScriptGuidFactories.clear();
		m_SceneScriptGuidFactories.clear();
		m_ViewScriptGuidFactories.clear();
#if Z_EDITOR
		m_ActiveInstances.clear();
		m_ActiveGameScripts.clear();
		m_ActiveSceneScripts.clear();
		m_ActiveViewScripts.clear();
#endif
	}

#if Z_EDITOR
	void ScriptStorage::RegisterInstance(Script* instance)       { RegisterInstanceImpl(instance, m_ActiveInstances); }
	void ScriptStorage::UnregisterInstance(Script* instance)     { UnregisterInstanceImpl(instance, m_ActiveInstances); }
	std::span<Script* const> ScriptStorage::GetActiveInstances() const { return m_ActiveInstances; }

	void ScriptStorage::RegisterInstance(GameScript* instance)   { RegisterInstanceImpl(instance, m_ActiveGameScripts); }
	void ScriptStorage::UnregisterInstance(GameScript* instance) { UnregisterInstanceImpl(instance, m_ActiveGameScripts); }
	std::span<GameScript* const> ScriptStorage::GetActiveGameScripts() const { return m_ActiveGameScripts; }

	void ScriptStorage::RegisterInstance(SceneScript* instance)  { RegisterInstanceImpl(instance, m_ActiveSceneScripts); }
	void ScriptStorage::UnregisterInstance(SceneScript* instance){ UnregisterInstanceImpl(instance, m_ActiveSceneScripts); }
	std::span<SceneScript* const> ScriptStorage::GetActiveSceneScripts() const { return m_ActiveSceneScripts; }

	void ScriptStorage::RegisterInstance(ViewScript* instance)   { RegisterInstanceImpl(instance, m_ActiveViewScripts); }
	void ScriptStorage::UnregisterInstance(ViewScript* instance) { UnregisterInstanceImpl(instance, m_ActiveViewScripts); }
	std::span<ViewScript* const> ScriptStorage::GetActiveViewScripts() const { return m_ActiveViewScripts; }
#endif
}
