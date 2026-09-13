#pragma once

#include <span>
#include <array>
#include <vector>
#include "math/utils/Types.h"
#include "core/utils/Ensure.h"

namespace zzz::core
{
	/**
	 * @class BitTreeTracker
	 * @brief 64-арное иерархическое битовое дерево изменений (Hierarchical Bitset).
	 *
	 * @details Предназначено для сверхбыстрой пометки (O(1)) и обхода установленных бит
	 * без холостых циклов за счет CPU-инструкции std::countr_zero.
	 *
	 * Размещение пирамиды:
	 * - Число слов каждого уровня рассчитывается строго от фактической емкости capacity (без скачков на границах 64^k).
	 * - Уровни расположены в едином плоском массиве m_Words.
	 * - Уровень 0: листья.
	 * - Уровень m_Depth - 1: корень (1 слово).
	 * - Метод ConsumeDirtyIndices() собирает грязные индексы и атомарно очищает посещённые биты без кадровых аллокаций.
	 */
	class BitTreeTracker final
	{
	public:
		static constexpr size_t kMaxDepth = 7;

		explicit BitTreeTracker(zU32 capacity = 1);

		/// @brief Резервирует емкость под заданное число элементов. Не сбрасывает выставленные биты, если размер не изменился.
		void Resize(zU32 capacity);

		/// @brief Сбрасывает все выставленные биты в 0.
		void Clear() noexcept;

		/// @brief Подготовка трекера: гарантирует емкость и очищает состояние.
		void Prepare(zU32 capacity);

		/// @brief Установка бита по индексу (помечает узел и каскадно поднимает 1 до корня).
		void Set(zU32 index) noexcept;

		/// @brief Проверяет, выставлен ли бит по индексу на листовом уровне.
		[[nodiscard]] inline bool IsSet(zU32 index) const noexcept
		{
			ensure(index < m_Capacity, "BitTreeTracker::IsSet: index must be less than capacity");

			const size_t wordGlobal = m_LevelOffsets[0] + (static_cast<size_t>(index) >> 6);
			const zU64 bitMask = 1ULL << (static_cast<size_t>(index) & 63ULL);

			return (m_Words[wordGlobal] & bitMask) != 0ULL;
		}

		/// @brief Извлекает все грязные индексы по возрастанию и очищает посещённые биты (0 аллокаций).
		/// @return Span на внутренний буфер трекера. Повторный вызов без новых Set() возвращает пустой span.
		[[nodiscard]] std::span<const zU32> ConsumeDirtyIndices();

		/// @brief Проверяет, есть ли хотя бы один грязный бит.
		[[nodiscard]] inline bool IsDirty() const noexcept { return m_IsDirty; }

		/// @brief Возвращает текущую емкость трекера.
		[[nodiscard]] inline zU32 GetCapacity() const noexcept { return m_Capacity; }

	private:
		void TraverseAndConsumeLevel(zU32 level, size_t wordIndexInLevel);

		zU32 m_Capacity{ 0 };
		zU32 m_Depth{ 1 };
		bool m_IsDirty{ false };

		std::array<size_t, kMaxDepth> m_LevelOffsets{};
		std::array<size_t, kMaxDepth> m_LevelWordCounts{};

		std::vector<zU64> m_Words;
		std::vector<zU32> m_DirtyIndices;
	};
}
