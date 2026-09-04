#pragma once

#include "engine/scene/storage/ISceneStorage.h"
#include <unordered_set>
#include <vector>

namespace zzz
{
	/**
	 * @class DefaultSceneStorage
	 * @brief Базовое линейное хранилище объектов сцены (без пространственного отсечения).
	 *
	 * @details Реализует строгое O(1) добавление и O(1) удаление через std::unordered_set.
	 * Отдает все зарегистрированные объекты через GetAll. Служит эталоном для последующих AABB/BVH.
	 */
	class DefaultSceneStorage final : public ISceneStorage
	{
	public:
		DefaultSceneStorage() = default;
		~DefaultSceneStorage() override = default;

		void Insert(GameObject* obj) override;
		void Remove(GameObject* obj) override;
		void Update(GameObject* obj) override;
		void Clear() override;

		void GetAll(std::vector<GameObject*>& outAll) const override;

		[[nodiscard]] size_t GetCount() const noexcept { return m_Objects.size(); }

	private:
		std::unordered_set<GameObject*> m_Objects;
	};
}
