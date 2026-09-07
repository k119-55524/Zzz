
#include "engine/scene/storage/DefaultSpatialStorage.h"

namespace zzz::engine
{
	SpatialHandle DefaultSpatialStorage::Insert(uint64_t userData)
	{
		uint32_t index = 0;
		if (!m_FreeIndices.empty())
		{
			index = m_FreeIndices.back();
			m_FreeIndices.pop_back();
			m_Slots[index].userData = userData;
			m_Slots[index].isOccupied = true;
		}
		else
		{
			index = static_cast<uint32_t>(m_Slots.size());
			m_Slots.push_back(Slot{ userData, true });
		}

		++m_ActiveCount;
		return index;
	}

	void DefaultSpatialStorage::Remove(SpatialHandle handle)
	{
		if (handle < m_Slots.size() && m_Slots[handle].isOccupied)
		{
			m_Slots[handle].isOccupied = false;
			m_Slots[handle].userData = 0;
			m_FreeIndices.push_back(handle);
			if (m_ActiveCount > 0)
			{
				--m_ActiveCount;
			}
		}
	}

	void DefaultSpatialStorage::Clear()
	{
		m_Slots.clear();
		m_FreeIndices.clear();
		m_ActiveCount = 0;
	}

	void DefaultSpatialStorage::GetAll(std::vector<uint64_t>& outUserData) const
	{
		outUserData.clear();
		outUserData.reserve(m_ActiveCount);
		for (const auto& slot : m_Slots)
		{
			if (slot.isOccupied)
			{
				outUserData.push_back(slot.userData);
			}
		}
	}
}
