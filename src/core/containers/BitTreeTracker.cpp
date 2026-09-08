
#include <array>
#include <algorithm>

#include "core/containers/BitTreeTracker.h"

namespace zzz::core
{
	BitTreeTracker::BitTreeTracker(uint32_t capacity) :
		m_Capacity{ capacity },
		m_Depth{ 1 },
		m_IsDirty{ false },
		m_Words{ 0 }
	{
		Prepare(capacity);
	}

	void BitTreeTracker::Prepare(zU32 capacity)
	{
		if (capacity == 0)
			capacity = 1;

		constexpr zU64 maxCapacity =
			(kLevelOffsets.back() - kLevelOffsets[kLevelOffsets.size() - 2]) * 64ULL;
		if (static_cast<zU64>(capacity) > maxCapacity)
			THROW_RUNTIME("BitTreeTracker::Prepare: capacity exceeds supported tree depth");

		m_Capacity = capacity;
		m_DirtyIndices.clear();
		if (m_DirtyIndices.capacity() < capacity)
			m_DirtyIndices.reserve(capacity);

		const size_t leafWordsNeeded = (capacity + 63ULL) >> 6;
		const size_t currentLeafCapacity =
			kLevelOffsets[m_Depth] - kLevelOffsets[m_Depth - 1];

		// Если емкости уже достаточно под элементы
		if (currentLeafCapacity >= leafWordsNeeded)
		{
			if (!m_IsDirty)
				return;

			std::fill(m_Words.begin(), m_Words.end(), 0ULL);
			m_IsDirty = false;

			return;
		}

		// Выбираем минимальную глубину пирамиды под возросшее число объектов
		m_Depth = 1;
		for (size_t depth = 1; depth < kLevelOffsets.size(); ++depth)
		{
			const size_t leafWordCapacity = kLevelOffsets[depth] - kLevelOffsets[depth - 1];
			if (leafWordsNeeded <= leafWordCapacity)
			{
				m_Depth = static_cast<zU32>(depth);
				break;
			}
		}

		m_Words.assign(kLevelOffsets[m_Depth], 0ULL);
		m_IsDirty = false;
	}

	void BitTreeTracker::GrowCapacity(zU32 newCapacity)
	{
		if (newCapacity <= m_Capacity)
		{
			return;
		}

		constexpr zU64 maxCapacity =
			(kLevelOffsets.back() - kLevelOffsets[kLevelOffsets.size() - 2]) * 64ULL;
		if (static_cast<zU64>(newCapacity) > maxCapacity)
			THROW_RUNTIME("BitTreeTracker::GrowCapacity: capacity exceeds supported tree depth");

		const size_t leafWordsNeeded = (newCapacity + 63ULL) >> 6;
		const size_t currentLeafCapacity =
			kLevelOffsets[m_Depth] - kLevelOffsets[m_Depth - 1];

		m_Capacity = newCapacity;

		// Если текущей пирамиды достаточно под возросшее число объектов
		if (currentLeafCapacity >= leafWordsNeeded)
		{
			return;
		}

		// Нужно увеличить глубину пирамиды, сохранив уже выставленные dirty-индексы
		std::vector<zU32> preservedIndices;
		if (m_IsDirty)
		{
			auto dirtySpan = GetDirtyIndices();
			preservedIndices.assign(dirtySpan.begin(), dirtySpan.end());
		}

		for (size_t depth = m_Depth + 1; depth < kLevelOffsets.size(); ++depth)
		{
			const size_t leafWordCapacity = kLevelOffsets[depth] - kLevelOffsets[depth - 1];
			if (leafWordsNeeded <= leafWordCapacity)
			{
				m_Depth = static_cast<zU32>(depth);
				break;
			}
		}

		m_Words.assign(kLevelOffsets[m_Depth], 0ULL);
		m_IsDirty = false;

		// Восстанавливаем ранее накопленные биты
		for (zU32 idx : preservedIndices)
		{
			Set(idx);
		}
	}

	void BitTreeTracker::Set(zU32 index) noexcept
	{
		ensure(index < m_Capacity, "BitTreeTracker::Set: index must be less than capacity");

		m_IsDirty = true;

		size_t wordInLevel = static_cast<size_t>(index) >> 6;
		size_t bitInWord = static_cast<size_t>(index) & 63ULL;

		for (zU32 lvl = 0; lvl < m_Depth; ++lvl)
		{
			const size_t wordGlobal = GetLevelOffset(lvl) + wordInLevel;
			const zU64 bitMask = 1ULL << bitInWord;
			const zU64 oldVal = m_Words[wordGlobal];

			if ((oldVal & bitMask) != 0ULL)
				break;

			m_Words[wordGlobal] = oldVal | bitMask;

			bitInWord = wordInLevel & 63ULL;
			wordInLevel >>= 6;
		}
	}

	std::span<const zU32> BitTreeTracker::GetDirtyIndices()
	{
		if (!m_IsDirty)
			return {};

		m_DirtyIndices.clear();
		TraverseLevel(m_Depth - 1, 0);

		return m_DirtyIndices;
	}

	void BitTreeTracker::TraverseLevel(zU32 level, size_t wordIndexInLevel)
	{
		const size_t wordGlobalIndex = GetLevelOffset(level) + wordIndexInLevel;
		zU64 mask = m_Words[wordGlobalIndex];

		if (level == 0)
		{
			const zU32 baseIndex = static_cast<zU32>(wordIndexInLevel << 6);
			while (mask != 0ULL)
			{
				const zU32 bit = static_cast<zU32>(std::countr_zero(mask));
				const zU32 itemIndex = baseIndex + bit;
				if (itemIndex < m_Capacity)
				{
					m_DirtyIndices.push_back(itemIndex);
				}
				mask &= (mask - 1ULL);
			}
		}
		else
		{
			const zU32 nextLevel = level - 1;
			const size_t nextWordBase = wordIndexInLevel << 6;

			while (mask != 0ULL)
			{
				const zU32 bit = static_cast<zU32>(std::countr_zero(mask));
				TraverseLevel(nextLevel, nextWordBase + bit);
				mask &= (mask - 1ULL);
			}
		}
	}
}
