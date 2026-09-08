#pragma once

#include <string>
#include <vector>
#include <span>
#include <cstddef>
#include <cstdint>
#include "core/utils/Guid.h"
#include "core/serialize/Serializer.h"
#include "core/enums/eObjectDomain.h"
#include "math/vector/Vec3.h"
#include "math/quat/Quat.h"

namespace zzz::core
{

	/**
	 * @class GameObjectData
	 * @brief Сериализуемое представление игрового объекта в package.dat / SceneData.
	 */
	class GameObjectData final : public ISerializable
	{
	public:
		GameObjectData() = default;
		GameObjectData(
			Guid guid,
			std::string name,
			eObjectDomain domain,
			bool isActive,
			math::Vec3<zF32> position,
			math::Quat<zF32> rotation,
			math::Vec3<zF32> scale,
			Guid meshGuid,
			Guid materialGuid,
			std::vector<Guid> scriptGuids,
			Guid parentGuid = Guid{});

		[[nodiscard]] const Guid& GetGuid() const noexcept { return m_Guid; }
		[[nodiscard]] const Guid& GetParentGuid() const noexcept { return m_ParentGuid; }
		[[nodiscard]] const std::string& GetName() const noexcept { return m_Name; }
		[[nodiscard]] eObjectDomain GetDomain() const noexcept { return m_Domain; }
		[[nodiscard]] bool IsEntity() const noexcept { return m_Domain == eObjectDomain::Entity; }
		[[nodiscard]] bool IsActive() const noexcept { return m_IsActive; }

		[[nodiscard]] const math::Vec3<zF32>& GetPosition() const noexcept { return m_Position; }
		[[nodiscard]] const math::Quat<zF32>& GetRotation() const noexcept { return m_Rotation; }
		[[nodiscard]] const math::Vec3<zF32>& GetScale() const noexcept { return m_Scale; }

		[[nodiscard]] const Guid& GetMeshGuid() const noexcept { return m_MeshGuid; }
		[[nodiscard]] const Guid& GetMaterialGuid() const noexcept { return m_MaterialGuid; }
		[[nodiscard]] const std::vector<Guid>& GetScriptGuids() const noexcept { return m_ScriptGuids; }

	protected:
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const Serializer& serializer) const override;
		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& serializer) override;

	private:
		Guid m_Guid;
		Guid m_ParentGuid;
		std::string m_Name;
		eObjectDomain m_Domain{ eObjectDomain::Object };
		bool m_IsActive{ true };

		math::Vec3<zF32> m_Position{ 0.0f, 0.0f, 0.0f };
		math::Quat<zF32> m_Rotation{ 0.0f, 0.0f, 0.0f, 1.0f };
		math::Vec3<zF32> m_Scale{ 1.0f, 1.0f, 1.0f };

		Guid m_MeshGuid;
		Guid m_MaterialGuid;

		std::vector<Guid> m_ScriptGuids;
	};
}
