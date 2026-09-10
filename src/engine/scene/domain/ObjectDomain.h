#pragma once

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

#include "core/utils/Guid.h"
#include "core/enums/eLayerType.h"
#include "core/utils/macros/MiscMacros.h"
#include "engine/scene/domain/IObjectDomain.h"
#include "engine/scene/gameobject/GameObject.h"

using namespace zzz::core;

namespace zzz::engine
{
	/**
	 * @class ObjectDomain
	 * @brief Реализация IObjectDomain для управления объектами сцены (GameObject).
	 */
	class ObjectDomain final : public IObjectDomain
	{
	public:
		ObjectDomain();
		~ObjectDomain() override;

		Z_NO_COPY_MOVE(ObjectDomain);

		void Update(float dt) override;
		void Clear() override;

		GameObject* AddObject(const Guid& guid, std::string name) override;
		[[nodiscard]] GameObject* FindObjectByGuid(const Guid& guid) const noexcept override;
		void GetAllObjects(std::vector<GameObject*>& outObjects) const override;

		[[nodiscard]] size_t GetObjectCount() const noexcept { return m_Objects.size(); }

	private:
		std::unordered_map<Guid, std::unique_ptr<GameObject>> m_Objects;
	};
}
