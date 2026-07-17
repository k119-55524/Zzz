#pragma once

#include <memory>
#include <vector>
#include <string>
#include <functional>
#include <string_view>
#include <type_traits>
#include <unordered_map>

#include <common/memory_utils.h>

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

#pragma warning(push)
#pragma warning(disable: 4251)
	class Z_ENGINE_API ScriptRegistry
	{
	public:
		// Регистрация типов
		template<typename T>
		static void Register(std::string_view name)
		{
			std::string nameStr(name);
			if constexpr (std::is_base_of_v<GameScript, T>)
			{
				s_GameScriptFactories[nameStr] = []()
				{
					return zzz::common::safe_make_shared<T>();
				};
			}
			else if constexpr (std::is_base_of_v<SceneScript, T>)
			{
				s_SceneScriptFactories[nameStr] = []()
				{
					return zzz::common::safe_make_shared<T>();
				};
			}
			else if constexpr (std::is_base_of_v<Script, T>)
			{
				s_ScriptFactories[nameStr] = [](GameObject* owner)
				{
					return zzz::common::safe_make_shared<T>(owner);
				};
			}
			else
			{
				static_assert(sizeof(T) == 0, "Unknown script base type");
			}
		}

		// Создание объектов
		static std::shared_ptr<Script> CreateScript(std::string_view name, GameObject* owner);
		static std::shared_ptr<GameScript> CreateGameScript(std::string_view name);
		static std::shared_ptr<SceneScript> CreateSceneScript(std::string_view name);

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
#endif

	private:
		static std::unordered_map<std::string, std::function<std::shared_ptr<Script>(GameObject*)>> s_ScriptFactories;
		static std::unordered_map<std::string, std::function<std::shared_ptr<GameScript>()>> s_GameScriptFactories;
		static std::unordered_map<std::string, std::function<std::shared_ptr<SceneScript>()>> s_SceneScriptFactories;

#if Z_EDITOR
		static std::vector<Script*> s_ActiveInstances;
		static std::vector<GameScript*> s_ActiveGameScripts;
		static std::vector<SceneScript*> s_ActiveSceneScripts;
#endif
	};
#pragma warning(pop)
}
