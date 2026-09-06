#pragma once

#include <cstdint>
#include <string_view>
#include <optional>

namespace zzz::core
{
	/**
	 * @enum eObjectDomain
	 * @brief Домен сущности на сцене: классический ООП-объект или ECS-сущность.
	 */
	enum class eObjectDomain : uint8_t
	{
		Object = 0, ///< Классический GameObject с иерархией Transform и собственными скриптами
		Entity = 1  ///< Высокоскоростная пакетная сущность для EntityWorld
	};

	[[nodiscard]] constexpr std::string_view ToString(eObjectDomain domain) noexcept
	{
		switch (domain)
		{
		case eObjectDomain::Object: return "Object";
		case eObjectDomain::Entity: return "Entity";
		}
		return "Unknown";
	}

	[[nodiscard]] constexpr std::optional<eObjectDomain> ParseObjectDomain(std::string_view str) noexcept
	{
		if (str == "Object") return eObjectDomain::Object;
		if (str == "Entity") return eObjectDomain::Entity;
		return std::nullopt;
	}
}
