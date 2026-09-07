#pragma once

#include <span>
#include <vector>
#include <cstdint>

namespace zzz::core
{
	/**
	 * @class BitTreeTracker
	 * @brief 64-арное иерархическое битовое дерево изменений (Hierarchical Bitset).
	 *
	 * @details Предназначено для сверхбыстрой пометки (O(1)), проверки (O(1)) и
	 * обхода установленных бит без холостых циклов за счет CPU-инструкции std::countr_zero.
	 *
	 * Архитектура чистой пирамиды:
	 * - Все уровни расположены сверху вниз в ЕДИНОМ плоском массиве m_Words.
	 * - Индекс 0: всегда корень (Уровень D-1, 1 слово uint64_t).
	 * - Далее идут промежуточные уровни (степени 64).
	 * - В конце располагается базовый уровень листьев (массив данных).
	 * - Буфер грязных индексов m_DirtyIndices живет внутри трекера и не требует аллокаций в кадре.
	 *
	 * Примеры вместимости по глубине пирамиды:
	 * - Глубина 1: 1 слово (до 64 элементов)
	 * - Глубина 2: 1 + 64 = 65 слов (до 4 096 элементов)
	 * - Глубина 3: 1 + 64 + 4 096 = 4 161 слово (до 262 144 элементов)
	 * - Глубина 4: 1 + 64 + 4 096 + 262 144 = 266 273 слова (до 16 777 216 элементов)
	 */
	class BitTreeTracker final
	{
	public:
		explicit BitTreeTracker(uint32_t initialCapacity = 1);

		/// @brief Подготовка трекера к кадру: гарантирует емкость под число элементов и сбрасывает все биты в 0.
		void Prepare(uint32_t capacity);

		/// @brief Установка бита по индексу (помечает узел и каскадно поднимает 1 до корня).
		void Set(uint32_t index) noexcept;

		/// @brief Возвращает span со всеми собранными грязными индексами текущего кадра (0 аллокаций).
		[[nodiscard]] std::span<const uint32_t> GetDirtyIndices();

	private:
		// Вычисляет смещение начала заданного уровня пирамиды (0 = листья, depth-1 = корень)
		[[nodiscard]] size_t GetLevelOffset(uint32_t level) const noexcept;

		void TraverseLevel(uint32_t level, size_t wordIndexInLevel);

		uint32_t              m_Capacity;
		uint32_t              m_Depth;
		bool                  m_IsDirty;
		std::vector<uint64_t> m_Words;
		std::vector<uint32_t> m_DirtyIndices;
	};
}
