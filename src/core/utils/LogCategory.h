#pragma once

#include <string_view>
#include "core/utils/Types.h"

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
	 *
	 * @details Категории объявляются как глобальные `inline constexpr` объекты (см. Z_DECLARE_LOG_CATEGORY /
	 * Z_DECLARE_GUARANTEED_LOG_CATEGORY ниже) и имеют стабильный на весь процесс адрес. LogEntry хранит указатель
	 * на категорию (`const LogCategory*`), а не копию строки её имени - это исключает лишние аллокации при логировании.
	 *
	 * `isGuaranteed` категории (LogGeneral, LogEngine, а также объявленные через
	 * Z_DECLARE_GUARANTEED_LOG_CATEGORY пользовательские категории) никогда не фильтруются рантаймом, даже
	 * на уровне Message - см. Logger::IsCategoryEnabled.
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
	 * @brief Объявляет фильтруемую рантаймом категорию логирования.
	 * @param Name  Имя объявляемой категории (используется и как идентификатор C++, и как отображаемое имя).
	 * @param Group Группа категории: Engine или User (без ::, см. eLogCategoryGroup).
	 */
#define Z_DECLARE_LOG_CATEGORY(Name, Group) \
	inline constexpr ::zzz::core::LogCategory Name{ #Name, ::zzz::core::eLogCategoryGroup::Group, false }

	/**
	 * @brief Объявляет гарантированную категорию логирования - никогда не фильтруется рантаймом.
	 * @param Name  Имя объявляемой категории.
	 * @param Group Группа категории: Engine или User (без ::, см. eLogCategoryGroup).
	 */
#define Z_DECLARE_GUARANTEED_LOG_CATEGORY(Name, Group) \
	inline constexpr ::zzz::core::LogCategory Name{ #Name, ::zzz::core::eLogCategoryGroup::Group, true }

	// --- Встроенные категории движка ---

	// Гарантированные - не подлежат рантайм-фильтрации ни при каких настройках (в т.ч. на уровне Message).
	Z_DECLARE_GUARANTEED_LOG_CATEGORY(LogGeneral, Engine); // Категория по умолчанию для файлов без Z_SET_LOG_CATEGORY(...).
	Z_DECLARE_GUARANTEED_LOG_CATEGORY(LogEngine, Engine);

	// Фильтруемые категории движка.
	Z_DECLARE_LOG_CATEGORY(LogGAPI, Engine);
	Z_DECLARE_LOG_CATEGORY(LogGAPIVerbose, Engine);
	Z_DECLARE_LOG_CATEGORY(LogECS, Engine);
	Z_DECLARE_LOG_CATEGORY(LogAudio, Engine);
	Z_DECLARE_LOG_CATEGORY(LogPhysics, Engine);
	Z_DECLARE_LOG_CATEGORY(LogAssets, Engine);
	Z_DECLARE_LOG_CATEGORY(LogNetwork, Engine);
	Z_DECLARE_LOG_CATEGORY(LogUI, Engine);
}
