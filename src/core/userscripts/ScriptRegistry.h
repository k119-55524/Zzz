#pragma once

#include <string>
#include <string_view>
#include <type_traits>

#include "core/utils/Guid.h"
#include "core/utils/Export.h"
#include "core/utils/MemoryUtils.h"
#include "ScriptStorage.h"

namespace zzz::core
{
	class ViewScript;
	class SceneScript;
	class GameScript;
	class Script;

	class Z_CORE_API ScriptRegistry
	{
	public:
		// Регистрация типов по имени
		template<typename T>
		static void Register(std::string_view name)
		{
			Register<T>(name, zzz::core::Guid{});
		}

		// Регистрация типов по имени и GUID
		template<typename T>
		static void Register(std::string_view name, const zzz::core::Guid& guid)
		{
			std::string nameStr(name);
			if constexpr (std::is_base_of_v<zzz::core::ViewScript, T>)
			{
				ScriptStorage::RegisterViewScriptFactory(nameStr, guid, []() { return zzz::core::safe_make_shared<T>(); });
			}
			else if constexpr (std::is_base_of_v<zzz::core::SceneScript, T>)
			{
				ScriptStorage::RegisterSceneScriptFactory(nameStr, guid, []() { return zzz::core::safe_make_shared<T>(); });
			}
			else if constexpr (std::is_base_of_v<zzz::core::GameScript, T>)
			{
				ScriptStorage::RegisterGameScriptFactory(nameStr, guid, []() { return zzz::core::safe_make_shared<T>(); });
			}
			else if constexpr (std::is_base_of_v<zzz::core::Script, T>)
			{
				ScriptStorage::RegisterScriptFactory(nameStr, guid, [](GameObject* owner) { return zzz::core::safe_make_shared<T>(owner); });
			}
			else
			{
				static_assert(sizeof(T) == 0, "Unknown script base type");
			}
		}

		static void Clear()
		{
			ScriptStorage::Clear();
		}
	};
}
