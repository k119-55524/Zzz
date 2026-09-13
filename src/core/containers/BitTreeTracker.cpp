#include "core/containers/BitTreeTracker.h"
#include <algorithm>
#include <bit>

namespace zzz::core
{
	BitTreeTracker::BitTreeTracker(zU32 capacity)
	{
		Prepare(capacity);
	}

	void BitTreeTracker::Resize(zU32 capacity)
	{
		if (capacity == 0)
			capacity = 1;

		if (capacity <= m_Capacity && !m_Words.empty())
		{
			m_Capacity = capacity;
			return;
		}

		m_Capacity = capacity;

		if (m_DirtyIndices.capacity() < capacity)
			m_DirtyIndices.reserve(capacity);

		// Вычисляем фактическое число слов на каждом уровне
		size_t wordsNeeded = (static_cast<size_t>(capacity) + 63ULL) >> 6;
		m_LevelWordCounts[0] = wordsNeeded;

		zU32 depth = 1;
		while (wordsNeeded > 1 && depth < kMaxDepth)
		{
			wordsNeeded = (wordsNeeded + 63ULL) >> 6;
			m_LevelWordCounts[depth] = wordsNeeded;
			depth++;
		}
		m_Depth = depth;

		// Смещения: корень (уровень m_Depth - 1) в начале
		size_t offset = 0;
		for (int lvl = static_cast<int>(m_Depth) - 1; lvl >= 0; --lvl)
		{
			m_LevelOffsets[lvl] = offset;
			offset += m_LevelWordCounts[lvl];
		}

		m_Words.assign(offset, 0ULL);
		m_IsDirty = false;
	}

	void BitTreeTracker::Clear() noexcept
	{
		if (m_IsDirty)
		{
			std::fill(m_Words.begin(), m_Words.end(), 0ULL);
			m_IsDirty = false;
		}
		m_DirtyIndices.clear();
	}

	void BitTreeTracker::Prepare(zU32 capacity)
	{
		Resize(capacity);
		Clear();
	}

	void BitTreeTracker::Set(zU32 index) noexcept
	{
		ensure(index < m_Capacity, "BitTreeTracker::Set: index must be less than capacity");

		m_IsDirty = true;

		size_t wordInLevel = static_cast<size_t>(index) >> 6;
		size_t bitInWord = static_cast<size_t>(index) & 63ULL;

		for (zU32 lvl = 0; lvl < m_Depth; ++lvl)
		{
			const size_t wordGlobal = m_LevelOffsets[lvl] + wordInLevel;
			const zU64 bitMask = 1ULL << bitInWord;
			const zU64 oldVal = m_Words[wordGlobal];

			if ((oldVal & bitMask) != 0ULL)
				break;

			m_Words[wordGlobal] = oldVal | bitMask;

			bitInWord = wordInLevel & 63ULL;
			wordInLevel >>= 6;
		}
	}

	std::span<const zU32> BitTreeTracker::ConsumeDirtyIndices()
	{
		if (!m_IsDirty)
			return {};

		m_DirtyIndices.clear();
		TraverseAndConsumeLevel(m_Depth - 1, 0);
		m_IsDirty = false;

		return m_DirtyIndices;
	}

	void BitTreeTracker::TraverseAndConsumeLevel(zU32 level, size_t wordIndexInLevel)
	{
		const size_t wordGlobalIndex = m_LevelOffsets[level] + wordIndexInLevel;
		zU64 mask = m_Words[wordGlobalIndex];
		m_Words[wordGlobalIndex] = 0ULL;

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
				TraverseAndConsumeLevel(nextLevel, nextWordBase + bit);
				mask &= (mask - 1ULL);
			}
		}
	}
}
