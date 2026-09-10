#pragma once

#include <string>
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
		std::string   name;
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
		bool          isDirty{ false };
	};

	/**
	 * @class ISceneTreeAccessor
	 * @brief Абстрактный скрытый интерфейс доступа GameObject и подсистем к плоскому SoA-хранилищу сцены.
	 */
	class ISceneTreeAccessor
	{
	public:
		virtual ~ISceneTreeAccessor() = default;

		// --- Пространственные координаты (Local) ---
		virtual void SetLocalPosition(NodeHandle handle, const Vec3<zF32>& pos) = 0;
		[[nodiscard]] virtual const Vec3<zF32>& GetLocalPosition(NodeHandle handle) const = 0;

		virtual void SetLocalRotation(NodeHandle handle, const Quat<zF32>& rot) = 0;
		[[nodiscard]] virtual const Quat<zF32>& GetLocalRotation(NodeHandle handle) const = 0;

		virtual void SetLocalScale(NodeHandle handle, const Vec3<zF32>& scale) = 0;
		[[nodiscard]] virtual const Vec3<zF32>& GetLocalScale(NodeHandle handle) const = 0;

		// --- Мировая матрица (World) ---
		[[nodiscard]] virtual const Mat4<zF32>& GetWorldMatrix(NodeHandle handle) const = 0;

		// --- Топология и иерархия ---
		virtual void SetParent(NodeHandle child, NodeHandle parent, bool keepWorldTransform = true) = 0;
		[[nodiscard]] virtual NodeHandle GetParent(NodeHandle handle) const = 0;
		[[nodiscard]] virtual GameObject* GetNodeOwner(NodeHandle handle) const = 0;
		virtual void MarkDirty(NodeHandle handle) = 0;

		// --- Свойства узла (делегирование из GameObject) ---
		virtual void SetActive(NodeHandle handle, bool active) = 0;
		[[nodiscard]] virtual bool IsActive(NodeHandle handle) const = 0;

		virtual void SetName(NodeHandle handle, std::string name) = 0;
		[[nodiscard]] virtual const std::string& GetName(NodeHandle handle) const = 0;

		virtual void SetSpatialHandle(NodeHandle handle, SpatialHandle spHandle) = 0;
		[[nodiscard]] virtual SpatialHandle GetSpatialHandle(NodeHandle handle) const noexcept = 0;
	};
}
