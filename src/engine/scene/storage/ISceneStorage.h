#pragma once

#include <vector>

namespace zzz
{
	class GameObject;

	/**
	 * @class ISceneStorage
	 * @brief Абстракция пространственного хранения и выборки объектов сцены.
	 */
	class ISceneStorage
	{
	public:
		virtual ~ISceneStorage() = default;

		virtual void Insert(GameObject* obj) = 0;
		virtual void Remove(GameObject* obj) = 0;
		virtual void Update(GameObject* obj) = 0;
		virtual void Clear() = 0;

		virtual void GetAll(std::vector<GameObject*>& outAll) const = 0;
	};
}
