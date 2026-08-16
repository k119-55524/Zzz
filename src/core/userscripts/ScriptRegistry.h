#pragma once

#include <string>
#include <string_view>
#include <type_traits>

#include "core/utils/Guid.h"
#include "core/utils/Export.h"
#include "core/utils/MemoryUtils.h"
#include "core/utils/Macroses.h"
#include "ScriptStorage.h"

namespace zzz::core
{
	class ViewScript;
	class SceneScript;
	class GameScript;
	class Script;

	class Z_CORE_API ScriptRegistry final
	{
		Z_NO_COPY_MOVE(ScriptRegistry);

	public:
		ScriptRegistry() = delete;
		explicit ScriptRegistry(ScriptStorage& storage) : m_Storage(storage) {}

		// Регистрация типов по имени
		template<typename T>
		void Register(std::string_view name)
		{
			Register<T>(name, zzz::core::Guid{});
		}

		// Регистрация типов по имени и GUID
		template<typename T>
		void Register(std::string_view name, const zzz::core::Guid& guid)
		{
			std::string nameStr(name);
			if constexpr (std::is_base_of_v<zzz::core::ViewScript, T>)
			{
				m_Storage.RegisterViewScriptFactory(nameStr, guid, []() { return zzz::core::safe_make_shared<T>(); });
			}
			else if constexpr (std::is_base_of_v<zzz::core::SceneScript, T>)
			{
				m_Storage.RegisterSceneScriptFactory(nameStr, guid, []() { return zzz::core::safe_make_shared<T>(); });
			}
			else if constexpr (std::is_base_of_v<zzz::core::GameScript, T>)
			{
				m_Storage.RegisterGameScriptFactory(nameStr, guid, []() { return zzz::core::safe_make_shared<T>(); });
			}
			else if constexpr (std::is_base_of_v<zzz::core::Script, T>)
			{
				m_Storage.RegisterScriptFactory(nameStr, guid, [](GameObject* owner) { return zzz::core::safe_make_shared<T>(owner); });
			}
			else
			{
				static_assert(sizeof(T) == 0, "Unknown script base type");
			}
		}

		void Clear()
		{
			m_Storage.Clear();
		}

	private:
		ScriptStorage& m_Storage;
	};
}
