#pragma once

/**
 * @file LogCategoryConstants.h
 * @brief Встроенные категории логирования ядра игрового движка.
 *
 * @details Содержит макросы объявления категорий ядра движка и перечень всех
 *          стандартных встроенных категорий логирования (GAPI, ECS, Audio, Physics и т.д.).
 *
 * @note Используется в:
 *       - Системе логирования (DOut, WOut, EOut, IOut, FOut, VOut)
 *       - UserSettingsManager (фильтрация и opt-out категорий)
 *       - Всех подсистемах движка для структурированного логирования
 */

#include "core/CoreIncludes.h"
#include "core/utils/LogCategory.h"

namespace zzz::core
{
#pragma region Built-in Log Categories
#define Z_DECLARE_LOG_CATEGORY_ENGINE(Name) \
	inline constexpr LogCategory Name{ #Name, eLogCategoryGroup::Engine, false }

#define Z_DECLARE_GUARANTEED_LOG_CATEGORY_ENGINE(Name) \
	inline constexpr LogCategory Name{ #Name, eLogCategoryGroup::Engine, true }

	// Пример обязательной (GUARANTEED) категории логирования:
	// Z_DECLARE_GUARANTEED_LOG_CATEGORY_ENGINE(MyCriticalSystem);

	Z_DECLARE_LOG_CATEGORY_ENGINE(GAPI);
	Z_DECLARE_LOG_CATEGORY_ENGINE(ECS);
	Z_DECLARE_LOG_CATEGORY_ENGINE(Audio);
	Z_DECLARE_LOG_CATEGORY_ENGINE(Physics);
	Z_DECLARE_LOG_CATEGORY_ENGINE(Assets);
	Z_DECLARE_LOG_CATEGORY_ENGINE(Network);
	Z_DECLARE_LOG_CATEGORY_ENGINE(UI);
	Z_DECLARE_LOG_CATEGORY_ENGINE(Hardware);
	Z_DECLARE_LOG_CATEGORY_ENGINE(Window);
	Z_DECLARE_LOG_CATEGORY_ENGINE(Input);
	Z_DECLARE_LOG_CATEGORY_ENGINE(Scene);

	Z_DECLARE_LOG_CATEGORY_ENGINE(ScriptObject);
	Z_DECLARE_LOG_CATEGORY_ENGINE(ScriptGame);
	Z_DECLARE_LOG_CATEGORY_ENGINE(ScriptScene);
	Z_DECLARE_LOG_CATEGORY_ENGINE(ScriptView);

#undef Z_DECLARE_LOG_CATEGORY_ENGINE
#undef Z_DECLARE_GUARANTEED_LOG_CATEGORY_ENGINE
#pragma endregion // Built-in Log Categories
}
