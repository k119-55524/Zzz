#include "engine/scene/storage/DefaultSceneStorage.h"
#include <algorithm>

namespace zzz
{
	void DefaultSceneStorage::Insert(GameObject* obj)
	{
		if (obj != nullptr)
		{
			m_Objects.insert(obj);
		}
	}

	void DefaultSceneStorage::Remove(GameObject* obj)
	{
		if (obj != nullptr)
		{
			m_Objects.erase(obj);
		}
	}

	void DefaultSceneStorage::Update(GameObject* /*obj*/)
	{
		// В DefaultSceneStorage отсечения нет, обновление позиции не требует перестроения
	}

	void DefaultSceneStorage::Clear()
	{
		m_Objects.clear();
	}

	void DefaultSceneStorage::GetAll(std::vector<GameObject*>& outAll) const
	{
		outAll.insert(outAll.end(), m_Objects.begin(), m_Objects.end());
	}
}
