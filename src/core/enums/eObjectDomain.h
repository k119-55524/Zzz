#pragma once

#include <cstdint>
#include <string_view>
#include <optional>
#include "core/utils/ThrowWrappers.h"

namespace zzz::core
{
	/**
	 * @enum eObjectDomain
	 * @brief Домен сущности на сцене: классический ООП-объект или ECS-сущность.
	 */
	enum class eObjectDomain : uint8_t
	{
		Object = 0, ///< Классический GameObject с иерархией Transform и собственными скриптами
		Entity = 1, ///< Высокоскоростная пакетная сущность для EntityWorld
		MVVM   = 2  ///< Декларативные элементы/контролы MVVM интерфейса
	};

	[[nodiscard]] constexpr std::string_view ToString(eObjectDomain domain)
	{
		switch (domain)
		{
		case eObjectDomain::Object: return "Object";
		case eObjectDomain::Entity: return "Entity";
		case eObjectDomain::MVVM:   return "MVVM";
		}
		THROW_RUNTIME("Необработанный eObjectDomain");
	}

	[[nodiscard]] constexpr std::optional<eObjectDomain> ParseObjectDomain(std::string_view str) noexcept
	{
		if (str == "Object") return eObjectDomain::Object;
		if (str == "Entity") return eObjectDomain::Entity;
		if (str == "MVVM")   return eObjectDomain::MVVM;
		return std::nullopt;
	}
}
