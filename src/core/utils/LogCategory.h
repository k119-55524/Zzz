#pragma once

#include <string_view>
#include "math/utils/Types.h"

namespace zzz::core
{
	/**
	 * @brief Группа категории лог-сообщения.
	 * @details Используется как "мастер-переключатель": можно отключить/включить сразу все категории
	 * движка или сразу все пользовательские категории, не перечисляя их поименно.
	 */
	enum class eLogCategoryGroup : zU8
	{
		Engine = 0,
		User   = 1
	};

	/**
	 * @brief Категория лог-сообщения.
	 * @details Категории объявляются как глобальные `inline constexpr` объекты и имеют стабильный на весь процесс адрес.
	 * LogEntry хранит указатель на категорию (`const LogCategory*`), а не копию строки имени - это исключает лишние аллокации.
	 * 
	 * Встроенные категории движка объявлены в LogCategoryConstants.h, пользовательские - через Z_DECLARE_LOG_CATEGORY_USER.
	 */
	struct LogCategory
	{
		std::string_view name;
		eLogCategoryGroup group;
		bool isGuaranteed;

		constexpr bool IsEngine() const noexcept { return group == eLogCategoryGroup::Engine; }
		constexpr bool IsUser() const noexcept { return group == eLogCategoryGroup::User; }
		constexpr bool IsGuaranteed() const noexcept { return isGuaranteed; }
	};

	/**
	 * @brief Объявляет фильтруемую пользовательскую категорию.
	 * @param Name Имя категории (C++ идентификатор и отображаемое имя).
	 * @example Z_DECLARE_LOG_CATEGORY_USER(LogGameLogic);
	 */
#define Z_DECLARE_LOG_CATEGORY_USER(Name) \
	inline constexpr ::zzz::core::LogCategory Name{ #Name, ::zzz::core::eLogCategoryGroup::User, false }

	// встроенные категории (GUARANTEED)
	inline constexpr LogCategory LogGeneral{ "General", eLogCategoryGroup::Engine, true };
	inline constexpr LogCategory LogEngine{ "Engine", eLogCategoryGroup::Engine, true };
	inline constexpr LogCategory GAPIVerbose{ "GAPIVerbose", eLogCategoryGroup::Engine, true };
}
