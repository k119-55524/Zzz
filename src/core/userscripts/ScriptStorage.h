#pragma once
#include <span>

#include <memory>
#include <vector>
#include <string>
#include <functional>
#include <string_view>
#include <unordered_map>

#include "core/utils/Guid.h"
#include "core/utils/Export.h"
#include "core/utils/Macroses.h"

namespace zzz::engine
{
	class GameObject;
}

namespace zzz
{
	using GameObject = ::zzz::engine::GameObject;
}

namespace zzz::core
{
	class Script;
	class GameScript;
	class SceneScript;
	class ViewScript;

#pragma warning(push)
#pragma warning(disable: 4251)
	class Z_CORE_API ScriptStorage final
	{
		Z_NO_COPY_MOVE(ScriptStorage);

	public:
		using ScriptFactoryFunc = std::function<std::shared_ptr<Script>(GameObject*)>;
		using GameScriptFactoryFunc = std::function<std::shared_ptr<GameScript>()>;
		using SceneScriptFactoryFunc = std::function<std::shared_ptr<SceneScript>()>;
		using ViewScriptFactoryFunc = std::function<std::shared_ptr<ViewScript>()>;

		ScriptStorage() = default;
		~ScriptStorage() = default;

		// Регистрация фабрик
		void RegisterScriptFactory(const std::string& name, const zzz::core::Guid& guid, ScriptFactoryFunc factory);
		void RegisterGameScriptFactory(const std::string& name, const zzz::core::Guid& guid, GameScriptFactoryFunc factory);
		void RegisterSceneScriptFactory(const std::string& name, const zzz::core::Guid& guid, SceneScriptFactoryFunc factory);
		void RegisterViewScriptFactory(const std::string& name, const zzz::core::Guid& guid, ViewScriptFactoryFunc factory);

		// Создание инстансов по имени
		[[nodiscard]] std::shared_ptr<Script> CreateScript(std::string_view name, GameObject* owner) const;
		[[nodiscard]] std::shared_ptr<GameScript> CreateGameScript(std::string_view name) const;
		[[nodiscard]] std::shared_ptr<SceneScript> CreateSceneScript(std::string_view name) const;
		[[nodiscard]] std::shared_ptr<ViewScript> CreateViewScript(std::string_view name) const;

		// Создание инстансов по GUID
		[[nodiscard]] std::shared_ptr<Script> CreateScript(const zzz::core::Guid& guid, GameObject* owner) const;
		[[nodiscard]] std::shared_ptr<GameScript> CreateGameScript(const zzz::core::Guid& guid) const;
		[[nodiscard]] std::shared_ptr<SceneScript> CreateSceneScript(const zzz::core::Guid& guid) const;
		[[nodiscard]] std::shared_ptr<ViewScript> CreateViewScript(const zzz::core::Guid& guid) const;

		// Получение списков
		[[nodiscard]] std::vector<std::string> GetAllGameScriptNames() const;

		// Очистка
		void Clear();

#if Z_EDITOR
		void RegisterInstance(Script* instance);
		void UnregisterInstance(Script* instance);
		[[nodiscard]] std::span<Script* const> GetActiveInstances() const;

		void RegisterInstance(GameScript* instance);
		void UnregisterInstance(GameScript* instance);
		[[nodiscard]] std::span<GameScript* const> GetActiveGameScripts() const;

		void RegisterInstance(SceneScript* instance);
		void UnregisterInstance(SceneScript* instance);
		[[nodiscard]] std::span<SceneScript* const> GetActiveSceneScripts() const;

		void RegisterInstance(ViewScript* instance);
		void UnregisterInstance(ViewScript* instance);
		[[nodiscard]] std::span<ViewScript* const> GetActiveViewScripts() const;
#endif

	private:
		std::unordered_map<std::string, ScriptFactoryFunc> m_ScriptFactories;
		std::unordered_map<std::string, GameScriptFactoryFunc> m_GameScriptFactories;
		std::unordered_map<std::string, SceneScriptFactoryFunc> m_SceneScriptFactories;
		std::unordered_map<std::string, ViewScriptFactoryFunc> m_ViewScriptFactories;

		std::unordered_map<zzz::core::Guid, ScriptFactoryFunc> m_ScriptGuidFactories;
		std::unordered_map<zzz::core::Guid, GameScriptFactoryFunc> m_GameScriptGuidFactories;
		std::unordered_map<zzz::core::Guid, SceneScriptFactoryFunc> m_SceneScriptGuidFactories;
		std::unordered_map<zzz::core::Guid, ViewScriptFactoryFunc> m_ViewScriptGuidFactories;

#if Z_EDITOR
		std::vector<Script*> m_ActiveInstances;
		std::vector<GameScript*> m_ActiveGameScripts;
		std::vector<SceneScript*> m_ActiveSceneScripts;
		std::vector<ViewScript*> m_ActiveViewScripts;
#endif
	};
#pragma warning(pop)
}
