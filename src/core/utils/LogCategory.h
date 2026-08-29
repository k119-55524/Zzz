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
	 * @details Категории объявляются как глобальные `inline constexpr` объекты (см. макросы ниже)
	 * и имеют стабильный на весь процесс адрес. LogEntry хранит указатель на категорию (`const LogCategory*`),
	 * а не копию строки её имени - это исключает лишние аллокации при логировании.
	 *
	 * `isGuaranteed` категории (LogGeneral, LogEngine - встроенные гарантированные категории движка)
	 * никогда не фильтруются рантаймом, даже на уровне Message - см. Logger::IsCategoryEnabled.
	 * Пользовательские категории всегда фильтруемы.
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

	// ========== Встроенные категории движка (приватные макросы) ==========
	namespace detail
	{
#define Z_DECLARE_LOG_CATEGORY_ENGINE(Name) \
		inline constexpr ::zzz::core::LogCategory Name{ #Name, ::zzz::core::eLogCategoryGroup::Engine, false }

#define Z_DECLARE_GUARANTEED_LOG_CATEGORY_ENGINE(Name) \
		inline constexpr ::zzz::core::LogCategory Name{ #Name, ::zzz::core::eLogCategoryGroup::Engine, true }

		Z_DECLARE_GUARANTEED_LOG_CATEGORY_ENGINE(LogGeneral);
		Z_DECLARE_GUARANTEED_LOG_CATEGORY_ENGINE(LogEngine);

		Z_DECLARE_LOG_CATEGORY_ENGINE(LogGAPI);
		Z_DECLARE_LOG_CATEGORY_ENGINE(LogGAPIVerbose);
		Z_DECLARE_LOG_CATEGORY_ENGINE(LogECS);
		Z_DECLARE_LOG_CATEGORY_ENGINE(LogAudio);
		Z_DECLARE_LOG_CATEGORY_ENGINE(LogPhysics);
		Z_DECLARE_LOG_CATEGORY_ENGINE(LogAssets);
		Z_DECLARE_LOG_CATEGORY_ENGINE(LogNetwork);
		Z_DECLARE_LOG_CATEGORY_ENGINE(LogUI);

#undef Z_DECLARE_LOG_CATEGORY_ENGINE
#undef Z_DECLARE_GUARANTEED_LOG_CATEGORY_ENGINE
	}

	using detail::LogGeneral;
	using detail::LogEngine;
	using detail::LogGAPI;
	using detail::LogGAPIVerbose;
	using detail::LogECS;
	using detail::LogAudio;
	using detail::LogPhysics;
	using detail::LogAssets;
	using detail::LogNetwork;
	using detail::LogUI;

	/**
	 * @brief Объявляет фильтруемую пользовательскую категорию.
	 * @param Name Имя категории (C++ идентификатор и отображаемое имя).
	 * @example Z_DECLARE_LOG_CATEGORY_USER(LogGameLogic);
	 */
#define Z_DECLARE_LOG_CATEGORY_USER(Name) \
	inline constexpr ::zzz::core::LogCategory Name{ #Name, ::zzz::core::eLogCategoryGroup::User, false }
}
