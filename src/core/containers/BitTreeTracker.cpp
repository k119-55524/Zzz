#include "core/containers/BitTreeTracker.h"
#include <algorithm>

namespace zzz::core
{
	BitTreeTracker::BitTreeTracker(uint32_t initialCapacity)
		: m_Capacity(0)
		, m_Depth(1)
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

		const size_t leafWordsNeeded = (capacity + 63ULL) >> 6;

		// 1. Если емкости уже достаточно, просто быстро обнуляем биты текущего кадра
		size_t currentLeafCapacity = 1ULL << (static_cast<size_t>(m_Depth - 1) * 6);
		if (capacity > 0 && currentLeafCapacity >= leafWordsNeeded && !m_Words.empty())
		{
			std::fill(m_Words.begin(), m_Words.end(), 0ULL);
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
	}

	void BitTreeTracker::Set(uint32_t index) noexcept
	{
		if (index >= m_Capacity)
		{
			return;
		}

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


	size_t BitTreeTracker::GetDirtyIndices(std::vector<uint32_t>& outIndices) const
	{
		outIndices.clear();

		if (m_Words.empty() || m_Words[0] == 0ULL)
		{
			return 0;
		}

		TraverseLevel(m_Depth - 1, 0, outIndices);
		return outIndices.size();
	}

	void BitTreeTracker::TraverseLevel(uint32_t level, size_t wordIndexInLevel, std::vector<uint32_t>& outIndices) const
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
					outIndices.push_back(itemIndex);
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
				TraverseLevel(nextLevel, nextWordBase + bit, outIndices);
				mask &= (mask - 1ULL);
			}
		}
	}
}