#pragma once

#include <span>
#include <string>
#include <vector>
#include <cstddef>

#include "math/quat/Quat.h"
#include "core/utils/Guid.h"
#include "math/vector/Vec3.h"
#include "core/serialize/Serializer.h"

using namespace zzz::math;

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
			bool isEntity,
			bool isActive,
			Vec3<zF32> position,
			Quat<zF32> rotation,
			Vec3<zF32> scale,
			Guid meshGuid,
			Guid materialGuid,
			std::vector<Guid> scriptGuids,
			Guid parentGuid = Guid{});

		[[nodiscard]] const Guid& GetGuid() const noexcept { return m_Guid; }
		[[nodiscard]] const Guid& GetParentGuid() const noexcept { return m_ParentGuid; }
		[[nodiscard]] const std::string& GetName() const noexcept { return m_Name; }
		[[nodiscard]] bool IsEntity() const noexcept { return m_IsEntity; }
		[[nodiscard]] bool IsActive() const noexcept { return m_IsActive; }

		[[nodiscard]] const Vec3<zF32>& GetPosition() const noexcept { return m_Position; }
		[[nodiscard]] const Quat<zF32>& GetRotation() const noexcept { return m_Rotation; }
		[[nodiscard]] const Vec3<zF32>& GetScale() const noexcept { return m_Scale; }

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
		bool m_IsEntity{ false };
		bool m_IsActive{ true };

		Vec3<zF32> m_Position{ 0.0f, 0.0f, 0.0f };
		Quat<zF32> m_Rotation{ 0.0f, 0.0f, 0.0f, 1.0f };
		Vec3<zF32> m_Scale{ 1.0f, 1.0f, 1.0f };

		Guid m_MeshGuid;
		Guid m_MaterialGuid;

		std::vector<Guid> m_ScriptGuids;
	};
}
