#pragma once

#include <memory>
#include <vector>
#include <string>
#include <functional>
#include <string_view>
#include <type_traits>
#include <unordered_map>

namespace zzz
{
	class GameObject;
}

namespace zzz::script
{
	class Script;
	class Game;
	class Scene;

	class ScriptRegistry
	{
	public:
		// Регистрация типов
		template<typename T>
		static void Register(std::string_view name)
		{
			std::string nameStr(name);
			if constexpr (std::is_base_of_v<Game, T>)
			{
				s_GameFactories[nameStr] = []()
				{
					return std::make_shared<T>();
				};
			}
			else if constexpr (std::is_base_of_v<Scene, T>)
			{
				s_SceneFactories[nameStr] = []()
				{
					return std::make_shared<T>();
				};
			}
			else if constexpr (std::is_base_of_v<Script, T>)
			{
				s_ScriptFactories[nameStr] = [](GameObject* owner)
				{
					return std::make_shared<T>(owner);
				};
			}
			else
			{
				static_assert(sizeof(T) == 0, "Unknown script base type");
			}
		}

		// Создание объектов
		static std::shared_ptr<Script> CreateScript(std::string_view name, GameObject* owner);
		static std::shared_ptr<Game> CreateGame(std::string_view name);
		static std::shared_ptr<Scene> CreateScene(std::string_view name);

		// Очистка реестра фабрик
		static void Clear();

#if Z_EDITOR
		// Управление активными инстансами для Hot-Reload
		static void RegisterInstance(Script* instance);
		static void UnregisterInstance(Script* instance);
		static const std::vector<Script*>& GetActiveInstances();
#endif

	private:
		static std::unordered_map<std::string, std::function<std::shared_ptr<Script>(GameObject*)>> s_ScriptFactories;
		static std::unordered_map<std::string, std::function<std::shared_ptr<Game>()>> s_GameFactories;
		static std::unordered_map<std::string, std::function<std::shared_ptr<Scene>()>> s_SceneFactories;

#if Z_EDITOR
		static std::vector<Script*> s_ActiveInstances;
#endif
	};
}
