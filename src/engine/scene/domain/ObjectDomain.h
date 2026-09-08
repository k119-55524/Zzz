#pragma once

#include <unordered_map>
#include <memory>
#include <string>
#include <vector>
#include <span>
#include "core/utils/Guid.h"
#include "core/utils/macros/MiscMacros.h"
#include "core/templates/SlotMap.h"
#include "engine/scene/domain/IObjectDomain.h"
#include "engine/scene/gameobject/GameObject.h"

namespace zzz::engine
{
	/**
	 * @class ObjectDomain
	 * @brief Реализация IObjectDomain для управления объектами сцены (GameObject).
	 *
	 * @details Реализует стабильное владение объектами через std::unique_ptr и
	 * плотный кэш-локальный обход SlotMap<GameObject*>.
	 */
	class ObjectDomain final : public IObjectDomain
	{
	public:
		ObjectDomain();
		~ObjectDomain() override;

		Z_NO_COPY_MOVE(ObjectDomain);

		// --- ILayerDomain ---
		void Update(float dt) override;
		void Clear() override;

		// --- IObjectDomain ---
		::zzz::GameObject* CreateObject(const ::zzz::core::Guid& guid, std::string name) override;
		void DestroyObject(::zzz::GameObject* obj) override;
		[[nodiscard]] ::zzz::GameObject* FindObjectByGuid(const ::zzz::core::Guid& guid) const noexcept override;
		void GetAllObjects(std::vector<::zzz::GameObject*>& outObjects) const override;

		[[nodiscard]] size_t GetObjectCount() const noexcept;
		[[nodiscard]] std::span<::zzz::GameObject* const> GetObjects() const noexcept;

	private:
		std::unordered_map<::zzz::GameObject*, std::unique_ptr<::zzz::GameObject>> m_AllocatedObjects;
		std::unordered_map<::zzz::core::Guid, ::zzz::GameObject*> m_GuidToObject;
		::zzz::core::SlotMap<::zzz::GameObject*> m_ActiveObjects;
	};
}
