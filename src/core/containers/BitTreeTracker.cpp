
#include <limits>
#include <algorithm>
#include <stdexcept>

#include "core/containers/BitTreeTracker.h"

namespace zzz::core
{
	BitTreeTracker::BitTreeTracker(uint32_t initialCapacity) :
		m_Capacity{1},
		m_Depth{1},
		m_IsDirty{false},
		m_LevelOffsets{0},
		m_Words{0}
	{
		ensure(initialCapacity >= 1, "BitTreeTracker::BitTreeTracker: initialCapacity must be >= 1");

		Prepare(initialCapacity);
	}

	void BitTreeTracker::Prepare(zU32 capacity)
	{
		if (capacity == 0)
			throw std::runtime_error("BitTreeTracker::Prepare: capacity must be >= 1");

		m_Capacity = capacity;

		if (m_DirtyIndices.capacity() < capacity)
			m_DirtyIndices.reserve(capacity);

		const size_t leafWordsNeeded = (capacity + 63ULL) >> 6;
		size_t currentLeafCapacity = 1ULL << (static_cast<size_t>(m_Depth - 1) * 6);

		// 1. Если емкости уже достаточно под элементы
		if (currentLeafCapacity >= leafWordsNeeded)
		{
			// Если изменений вообще не было — выходим мгновенно за O(1)
			if (!m_IsDirty)
			{
				return;
			}

			// Если изменения были — сбрасываем биты текущего кадра
			std::fill(m_Words.begin(), m_Words.end(), 0ULL);
			m_IsDirty = false;
			return;
		}

		// 2. Рассчитываем новую глубину пирамиды под возросшее число объектов
		m_Depth = 1;
		currentLeafCapacity = 1;
		while (currentLeafCapacity < leafWordsNeeded)
		{
			currentLeafCapacity <<= 6;
			++m_Depth;
		}

		// 3. Вычисляем смещения и суммарный размер: sum = 1 + 64 + 64^2 + ...
		size_t totalWords = 0;
		size_t levelWords = 1;
		m_LevelOffsets.resize(m_Depth);
		for (uint32_t topDistance = 0; topDistance < m_Depth; ++topDistance)
		{
			ensure(totalWords <= std::numeric_limits<uint32_t>::max(),
				"BitTreeTracker::Prepare: level offset exceeds uint32_t range");

			const uint32_t level = m_Depth - 1 - topDistance;
			m_LevelOffsets[level] = static_cast<uint32_t>(totalWords);
			totalWords += levelWords;
			levelWords <<= 6;
		}

		// 4. Выделяем память с полным занулением
		m_Words.assign(totalWords, 0ULL);
		m_IsDirty = false;
	}

	void BitTreeTracker::Set(zU32 index) noexcept
	{
		ensure(index < m_Capacity,
			"BitTreeTracker::Set: index must be less than capacity");

		m_IsDirty = true;

		size_t wordInLevel = static_cast<size_t>(index) >> 6;
		size_t bitInWord = static_cast<size_t>(index) & 63ULL;

		for (zU32 lvl = 0; lvl < m_Depth; ++lvl)
		{
			const size_t wordGlobal = GetLevelOffset(lvl) + wordInLevel;
			const zU64 bitMask = 1ULL << bitInWord;
			const zU64 oldVal = m_Words[wordGlobal];

			m_Words[wordGlobal] = oldVal | bitMask;

			if ((oldVal & bitMask) != 0ULL)
			{
				break;
			}

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

	size_t BitTreeTracker::GetLevelOffset(zU32 level) const noexcept
	{
		return m_LevelOffsets[level];
	}
}
