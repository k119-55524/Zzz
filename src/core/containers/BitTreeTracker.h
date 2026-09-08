#pragma once

#include <span>
#include <array>
#include <vector>
#include "math/utils/Types.h"

namespace zzz::core
{
	inline constexpr std::array<size_t, 7> kLevelOffsets
	{
		0ULL,
		1ULL,
		65ULL,
		4161ULL,
		266305ULL,
		17043521ULL,
		1090785345ULL
	};

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
	 * - Глубина 4: 1 + 64 + 4 096 + 262 144 = 266 305 слов (до 16 777 216 элементов)
	 */
	class BitTreeTracker final
	{
	public:
		explicit BitTreeTracker(zU32 capacity = 1);

		/// @brief Подготовка трекера к кадру: гарантирует емкость под число элементов и сбрасывает все биты в 0. Нулевая емкость нормализуется до 1.
		void Prepare(zU32 capacity);

		/// @brief Установка бита по индексу (помечает узел и каскадно поднимает 1 до корня).
		/// @warning index < Capacity гарантируется вызывающей стороной (доверенный контракт, аналогично GUID из Правила 15): ensure ловит нарушение только в Debug/Dev, в Release проверка - zero-overhead no-op, защиты от выхода за границы m_Words нет.
		void Set(zU32 index) noexcept;

		/// @brief Возвращает span со всеми собранными грязными индексами текущего кадра (0 аллокаций). Безопасно вызывать повторно между вызовами Prepare() - результат не дублируется.
		/// @warning Возвращаемый span указывает на внутренний буфер трекера и валиден строго до следующего вызова Prepare() в текущем потоке. Порядок индексов - строго по возрастанию слотов SoA (не топологический).
		[[nodiscard]] std::span<const zU32> GetDirtyIndices();

	private:
		// Вычисляет смещение начала заданного уровня пирамиды (0 = листья, depth-1 = корень)
		[[nodiscard]] inline size_t GetLevelOffset(zU32 level) const noexcept { return kLevelOffsets[m_Depth - 1 - level]; }

		void TraverseLevel(zU32 level, size_t wordIndexInLevel);

		zU32 m_Capacity;
		zU32 m_Depth;
		bool m_IsDirty;
		std::vector<zU64> m_Words;
		std::vector<zU32> m_DirtyIndices;
	};
}
