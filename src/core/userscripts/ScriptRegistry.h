#pragma once

#include <memory>
#include <vector>
#include <string>
#include <functional>
#include <string_view>
#include <type_traits>
#include <unordered_map>

#include <core/utils/Guid.h>
#include <common/MemoryUtils.h>

#include "EngineExport.h"

namespace zzz
{
	class GameObject;
}

namespace zzz::script
{
	class Script;
	class GameScript;
	class SceneScript;
	class ViewScript;

#pragma warning(push)
#pragma warning(disable: 4251)
	class Z_ENGINE_API ScriptRegistry
	{
	public:
		// Регистрация типов по имени
		template<typename T>
		static void Register(std::string_view name)
		{
			Register<T>(name, zzz::common::Guid{});
		}

		// Регистрация типов по имени и GUID
		template<typename T>
		static void Register(std::string_view name, const zzz::common::Guid& guid)
		{
			std::string nameStr(name);
			if constexpr (std::is_base_of_v<zzz::script::ViewScript, T>)
			{
				auto factory = []() { return zzz::common::safe_make_shared<T>(); };
				s_ViewScriptFactories[nameStr] = factory;
				if (!guid.IsEmpty())
					s_ViewScriptGuidFactories[guid] = factory;
			}
			else if constexpr (std::is_base_of_v<zzz::script::SceneScript, T>)
			{
				auto factory = []() { return zzz::common::safe_make_shared<T>(); };
				s_SceneScriptFactories[nameStr] = factory;
				if (!guid.IsEmpty())
					s_SceneScriptGuidFactories[guid] = factory;
			}
			else if constexpr (std::is_base_of_v<zzz::script::GameScript, T>)
			{
				auto factory = []() { return zzz::common::safe_make_shared<T>(); };
				s_GameScriptFactories[nameStr] = factory;
				if (!guid.IsEmpty())
					s_GameScriptGuidFactories[guid] = factory;
			}
			else if constexpr (std::is_base_of_v<zzz::script::Script, T>)
			{
				auto factory = [](GameObject* owner) { return zzz::common::safe_make_shared<T>(owner); };
				s_ScriptFactories[nameStr] = factory;
				if (!guid.IsEmpty())
					s_ScriptGuidFactories[guid] = factory;
			}
			else
			{
				static_assert(sizeof(T) == 0, "Unknown script base type");
			}
		}

		// Создание объектов по имени
		static std::shared_ptr<Script> CreateScript(std::string_view name, GameObject* owner);
		static std::shared_ptr<GameScript> CreateGameScript(std::string_view name);
		static std::shared_ptr<SceneScript> CreateSceneScript(std::string_view name);
		static std::shared_ptr<ViewScript> CreateViewScript(std::string_view name);

		// Создание объектов по GUID
		static std::shared_ptr<Script> CreateScript(const zzz::common::Guid& guid, GameObject* owner);
		static std::shared_ptr<GameScript> CreateGameScript(const zzz::common::Guid& guid);
		static std::shared_ptr<SceneScript> CreateSceneScript(const zzz::common::Guid& guid);
		static std::shared_ptr<ViewScript> CreateViewScript(const zzz::common::Guid& guid);

		// Имена всех зарегистрированных глобальных (Game) скриптов - используется статической
		// сборкой игры для автостарта всех скриптов проекта (см. Engine::Initialize).
		static std::vector<std::string> GetAllGameScriptNames();

		// Очистка реестра фабрик
		static void Clear();

#if Z_EDITOR
		// Управление активными инстансами для Hot-Reload
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
		static std::unordered_map<std::string, std::function<std::shared_ptr<Script>(GameObject*)>> s_ScriptFactories;
		static std::unordered_map<std::string, std::function<std::shared_ptr<GameScript>()>> s_GameScriptFactories;
		static std::unordered_map<std::string, std::function<std::shared_ptr<SceneScript>()>> s_SceneScriptFactories;
		static std::unordered_map<std::string, std::function<std::shared_ptr<ViewScript>()>> s_ViewScriptFactories;

		static std::unordered_map<zzz::common::Guid, std::function<std::shared_ptr<Script>(GameObject*)>> s_ScriptGuidFactories;
		static std::unordered_map<zzz::common::Guid, std::function<std::shared_ptr<GameScript>()>> s_GameScriptGuidFactories;
		static std::unordered_map<zzz::common::Guid, std::function<std::shared_ptr<SceneScript>()>> s_SceneScriptGuidFactories;
		static std::unordered_map<zzz::common::Guid, std::function<std::shared_ptr<ViewScript>()>> s_ViewScriptGuidFactories;

#if Z_EDITOR
		static std::vector<Script*> s_ActiveInstances;
		static std::vector<GameScript*> s_ActiveGameScripts;
		static std::vector<SceneScript*> s_ActiveSceneScripts;
		static std::vector<ViewScript*> s_ActiveViewScripts;
#endif
	};
#pragma warning(pop)
}
