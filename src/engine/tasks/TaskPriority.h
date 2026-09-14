#pragma once

#include <cstdint>

namespace zzz::engine
{
	/**
	 * @enum eTaskPriority
	 * @brief Приоритеты задач для маршрутизации в пулы воркеров TaskDispatcher.
	 */
	enum class eTaskPriority : uint8_t
	{
		Critical = 0, // Кадровые задачи окон View, рендер, пре-рендер
		High = 1,     // Тяжелые задачи кадра (Culling, Animation, Geometry Prep)
		Normal = 2,   // Загрузка сцен, слоёв, фоновая десериализация
		Background = 3, // Сборка мусора (GC), аналитика, вторичные задачи

		Count
	};
}
