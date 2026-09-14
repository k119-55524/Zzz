#pragma once

#include <cstdint>
#include <string_view>
#include "core/utils/ThrowWrappers.h"

namespace zzz::engine
{
	/**
	 * @enum eTaskPriority
	 * @brief Приоритеты задач для маршрутизации в пулы воркеров TaskDispatcher.
	 */
	enum class eTaskPriority : uint8_t
	{
		Critical = 0,	// Кадровые задачи окон View, рендер, пре-рендер
		High = 1,		// Тяжелые задачи кадра (Culling, Animation, Geometry Prep)
		Normal = 2,		// Загрузка сцен, слоёв, фоновая десериализация
		Background = 3,	// Сборка мусора (GC), аналитика, вторичные задачи
		Count = 4
	};

	[[nodiscard]] constexpr std::string_view ToString(eTaskPriority priority)
	{
		switch (priority)
		{
		case eTaskPriority::Critical:   return "Critical";
		case eTaskPriority::High:       return "High";
		case eTaskPriority::Normal:     return "Normal";
		case eTaskPriority::Background: return "Background";
		case eTaskPriority::Count:      return "Count";
		}
		THROW_RUNTIME("Необработанный eTaskPriority");
	}
}
