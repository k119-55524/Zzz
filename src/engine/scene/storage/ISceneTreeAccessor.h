#pragma once

#include <cstdint>

#include "math/quat/Quat.h"
#include "math/vector/Vec3.h"
#include "math/matrix/Mat4.h"

using namespace zzz::math;

namespace zzz::engine
{
	class GameObject;

	using SpatialHandle = uint32_t;

	enum class SceneNodeType : uint8_t
	{
		Empty,
		GameObject,
		Entity
	};

	struct NodeHandle
	{
		uint32_t index{ 0xFFFFFFFF };
		uint32_t generation{ 0 };

		/// @brief Проверка на ненулевой дескриптор (полную проверку с generation выполняет SceneTreeContainer::IsValid).
		[[nodiscard]] constexpr bool IsValid() const noexcept { return index != 0xFFFFFFFF; }

		[[nodiscard]] constexpr bool operator==(const NodeHandle& other) const noexcept
		{
			return index == other.index && generation == other.generation;
		}

		[[nodiscard]] constexpr bool operator!=(const NodeHandle& other) const noexcept
		{
			return !(*this == other);
		}
	};

	struct NodeTopology
	{
		uint32_t parentIndex{ 0xFFFFFFFF };
		uint32_t firstChildIndex{ 0xFFFFFFFF };
		uint32_t nextSiblingIndex{ 0xFFFFFFFF };
		uint32_t prevSiblingIndex{ 0xFFFFFFFF };
	};

	struct LocalTransform
	{
		Vec3<zF32> position{ 0.0f, 0.0f, 0.0f };
		Quat<zF32> rotation{ 0.0f, 0.0f, 0.0f, 1.0f };
		Vec3<zF32> scale{ 1.0f, 1.0f, 1.0f };
	};

	struct NodeMetadata
	{
		SceneNodeType payloadType{ SceneNodeType::Empty };
		union
		{
			GameObject* gameObject{ nullptr };
			uint32_t    entityId;
		};

		SpatialHandle spatialHandle{ 0xFFFFFFFF };
		uint32_t      generation{ 1 };
		bool          isAlive{ true };
		bool          isActive{ true };
	};
}
