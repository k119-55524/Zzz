#pragma once

#include <cstdint>
#include <string_view>

namespace zzz::core
{
	/**
	 * @enum eThreadPriority
	 * @brief Платформонезависимые уровни приоритета потоков операционной системы.
	 */
	enum class eThreadPriority : uint8_t
	{
		Critical = 0,   // Кадровый рендер и пре-рендер (наивысший приоритет)
		High = 1,       // Тяжелые задачи кадра (анимации, куллинг)
		Normal = 2,     // Основные рабочие задачи (стандартный приоритет по умолчанию)
		Background = 3, // Фоновые задачи (GC, аналитика, минимальный приоритет)
		Count = 4
	};

	/**
	 * @brief Устанавливает имя текущего потока в операционной системе и отладчике.
	 * @param name Имя потока (UTF-8).
	 */
	void SetCurrentThreadName(std::string_view name);

	/**
	 * @brief Устанавливает приоритет планировщика ОС для текущего потока.
	 * @param priority Уровень приоритета потока.
	 */
	void SetCurrentThreadPriority(eThreadPriority priority);
}
