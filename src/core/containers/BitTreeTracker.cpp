
#include <algorithm>

#include "core/containers/BitTreeTracker.h"

namespace zzz::core
{
	BitTreeTracker::BitTreeTracker(uint32_t initialCapacity)
		: m_Capacity(0)
		, m_Depth(1)
		, m_IsDirty(false)
		, m_Words{ 0ULL }
		, m_DirtyIndices{}
	{
		Prepare(initialCapacity);
	}

	size_t BitTreeTracker::GetLevelOffset(uint32_t level) const noexcept
	{
		if (level >= m_Depth - 1)
		{
			return 0;
		}

		const uint32_t topDistance = (m_Depth - 1) - level;
		size_t offset = 0;
		size_t levelSize = 1;
		for (uint32_t i = 0; i < topDistance; ++i)
		{
			offset += levelSize;
			levelSize <<= 6;
		}
		return offset;
	}

	void BitTreeTracker::Prepare(uint32_t capacity)
	{
		m_Capacity = capacity;

		if (m_DirtyIndices.capacity() < capacity)
		{
			m_DirtyIndices.reserve(capacity);
		}

		const size_t leafWordsNeeded = (capacity + 63ULL) >> 6;
		size_t currentLeafCapacity = 1ULL << (static_cast<size_t>(m_Depth - 1) * 6);

		// 1. Если емкости уже достаточно под элементы
		if (capacity > 0 && currentLeafCapacity >= leafWordsNeeded && !m_Words.empty())
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

		// 3. Вычисляем суммарный размер всех уровней: sum = 1 + 64 + 64^2 + ...
		size_t totalWords = 0;
		size_t levelWords = 1;
		for (uint32_t i = 0; i < m_Depth; ++i)
		{
			totalWords += levelWords;
			levelWords <<= 6;
		}

		// 4. Выделяем память с полным занулением
		m_Words.assign(totalWords, 0ULL);
		m_IsDirty = false;
	}

	void BitTreeTracker::Set(uint32_t index) noexcept
	{
		if (index >= m_Capacity)
		{
			return;
		}

		m_IsDirty = true;

		size_t wordInLevel = static_cast<size_t>(index) >> 6;
		size_t bitInWord = static_cast<size_t>(index) & 63ULL;

		for (uint32_t lvl = 0; lvl < m_Depth; ++lvl)
		{
			const size_t wordGlobal = GetLevelOffset(lvl) + wordInLevel;
			const uint64_t bitMask = 1ULL << bitInWord;
			const uint64_t oldVal = m_Words[wordGlobal];

			m_Words[wordGlobal] = oldVal | bitMask;

			if ((oldVal & bitMask) != 0ULL)
			{
				break;
			}

			bitInWord = wordInLevel & 63ULL;
			wordInLevel >>= 6;
		}
	}

	std::span<const uint32_t> BitTreeTracker::GetDirtyIndices()
	{
		m_DirtyIndices.clear();

		if (!m_IsDirty)
		{
			return {};
		}

		TraverseLevel(m_Depth - 1, 0);

		return m_DirtyIndices;
	}

	void BitTreeTracker::TraverseLevel(uint32_t level, size_t wordIndexInLevel)
	{
		const size_t wordGlobalIndex = GetLevelOffset(level) + wordIndexInLevel;
		uint64_t mask = m_Words[wordGlobalIndex];

		if (level == 0)
		{
			const uint32_t baseIndex = static_cast<uint32_t>(wordIndexInLevel << 6);
			while (mask != 0ULL)
			{
				const uint32_t bit = static_cast<uint32_t>(std::countr_zero(mask));
				const uint32_t itemIndex = baseIndex + bit;
				if (itemIndex < m_Capacity)
				{
					m_DirtyIndices.push_back(itemIndex);
				}
				mask &= (mask - 1ULL);
			}
		}
		else
		{
			const uint32_t nextLevel = level - 1;
			const size_t nextWordBase = wordIndexInLevel << 6;

			while (mask != 0ULL)
			{
				const uint32_t bit = static_cast<uint32_t>(std::countr_zero(mask));
				TraverseLevel(nextLevel, nextWordBase + bit);
				mask &= (mask - 1ULL);
			}
		}
	}
}