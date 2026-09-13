#pragma once

#include <span>
#include <vector>
#include <cstdint>

#include "core/utils/Ensure.h"
#include "core/utils/macros/MiscMacros.h"
#include "core/containers/BitTreeTracker.h"
#include "engine/scene/storage/NodeTypes.h"

using namespace zzz::math;
using namespace zzz::core;

namespace zzz::core
{
	class GameObjectData;
}

namespace zzz::engine
{
	/**
	 * @class NodeStorage
	 * @brief Плоское SoA-хранилище (Structure of Arrays) данных узлов слоя сцены.
	 *
	 * @details Инкапсулирует параллельные непрерывные векторы топологии, локальных TRS,
	 * локальных и мировых матриц, метаданных узлов и битовое дерево изменений (BitTreeTracker).
	 */
	class NodeStorage final
	{
		Z_NO_COPY(NodeStorage);

	public:
		NodeStorage(NodeStorage&&) noexcept = default;
		NodeStorage& operator=(NodeStorage&&) noexcept = default;

		NodeStorage();
		explicit NodeStorage(std::span<const zzz::core::GameObjectData> objects);
		~NodeStorage() = default;

		std::span<const TransformChangeRange> ResolveTransforms();

#pragma region Getters and Setters
		// --- Пространственные координаты ---
		void SetLocalPosition(NodeHandle handle, const Vec3<zF32>& pos)
		{
			ensure(IsValid(handle), "NodeStorage::SetLocalPosition: невалидный handle");
			m_LocalTransforms[handle].position = pos;
			MarkDirty(handle);
		}

		[[nodiscard]] Vec3<zF32> GetLocalPosition(NodeHandle handle) const
		{
			ensure(IsValid(handle), "NodeStorage::GetLocalPosition: невалидный handle");
			return m_LocalTransforms[handle].position;
		}

		void SetLocalRotation(NodeHandle handle, const Quat<zF32>& rot)
		{
			ensure(IsValid(handle), "NodeStorage::SetLocalRotation: невалидный handle");
			m_LocalTransforms[handle].rotation = rot;
			MarkDirty(handle);
		}

		[[nodiscard]] Quat<zF32> GetLocalRotation(NodeHandle handle) const
		{
			ensure(IsValid(handle), "NodeStorage::GetLocalRotation: невалидный handle");
			return m_LocalTransforms[handle].rotation;
		}

		void SetLocalScale(NodeHandle handle, const Vec3<zF32>& scale)
		{
			ensure(IsValid(handle), "NodeStorage::SetLocalScale: невалидный handle");
			m_LocalTransforms[handle].scale = scale;
			MarkDirty(handle);
		}

		[[nodiscard]] Vec3<zF32> GetLocalScale(NodeHandle handle) const
		{
			ensure(IsValid(handle), "NodeStorage::GetLocalScale: невалидный handle");
			return m_LocalTransforms[handle].scale;
		}

		[[nodiscard]] Mat4<zF32> GetLocalMatrix(NodeHandle handle) const
		{
			ensure(IsValid(handle), "NodeStorage::GetLocalMatrix: невалидный handle");
			const auto& local = m_LocalTransforms[handle];
			return Mat4<zF32>::Scaling(local.scale) * local.rotation.ToMat4() * Mat4<zF32>::Translation(local.position);
		}

		[[nodiscard]] const Mat4<zF32>& GetWorldMatrix(NodeHandle handle) const
		{
			ensure(IsValid(handle), "NodeStorage::GetWorldMatrix: невалидный handle");
			return m_WorldMatrices[handle];
		}

		// --- Топология и иерархия ---
		[[nodiscard]] NodeHandle GetParent(NodeHandle handle) const
		{
			ensure(IsValid(handle), "NodeStorage::GetParent: невалидный handle");
			return m_ParentIndices[handle];
		}

		[[nodiscard]] zU32 GetSubtreeEnd(NodeHandle handle) const
		{
			ensure(IsValid(handle), "NodeStorage::GetSubtreeEnd: невалидный handle");
			return m_SubtreeEnds[handle];
		}

		void MarkDirty(NodeHandle handle)
		{
			ensure(IsValid(handle), "NodeStorage::MarkDirty: индекс узла выходит за пределы хранилища");
			m_DirtyTracker.Set(handle);
		}

		// --- Свойства узла ---
		void SetActive(NodeHandle handle, bool active)
		{
			ensure(IsValid(handle), "NodeStorage::SetActive: невалидный handle");
			if (active)
			{
				m_Flags[handle] = static_cast<eNodeFlags>(static_cast<zU8>(m_Flags[handle]) | static_cast<zU8>(eNodeFlags::Active));
			}
			else
			{
				m_Flags[handle] = static_cast<eNodeFlags>(static_cast<zU8>(m_Flags[handle]) & ~static_cast<zU8>(eNodeFlags::Active));
			}
		}

		[[nodiscard]] bool IsActive(NodeHandle handle) const
		{
			ensure(IsValid(handle), "NodeStorage::IsActive: невалидный handle");
			return (static_cast<zU8>(m_Flags[handle]) & static_cast<zU8>(eNodeFlags::Active)) != 0;
		}

		void SetSpatialHandle(NodeHandle handle, SpatialHandle spHandle)
		{
			ensure(IsValid(handle), "NodeStorage::SetSpatialHandle: невалидный handle");
			m_Bindings[handle].spatialHandle = spHandle;
		}

		[[nodiscard]] SpatialHandle GetSpatialHandle(NodeHandle handle) const
		{
			ensure(IsValid(handle), "NodeStorage::GetSpatialHandle: невалидный handle");
			return m_Bindings[handle].spatialHandle;
		}

		void SetDomainBinding(NodeHandle handle, DomainHandle domainHandle, eNodeDomainKind kind)
		{
			ensure(IsValid(handle), "NodeStorage::SetDomainBinding: невалидный handle");
			m_Bindings[handle].domainHandle = domainHandle;
			m_Bindings[handle].domainKind = kind;
		}

		[[nodiscard]] DomainHandle GetDomainHandle(NodeHandle handle) const
		{
			ensure(IsValid(handle), "NodeStorage::GetDomainHandle: невалидный handle");
			return m_Bindings[handle].domainHandle;
		}

		[[nodiscard]] eNodeDomainKind GetDomainKind(NodeHandle handle) const
		{
			ensure(IsValid(handle), "NodeStorage::GetDomainKind: невалидный handle");
			return m_Bindings[handle].domainKind;
		}

		// --- Валидация ---
		[[nodiscard]] bool IsValid(NodeHandle handle) const noexcept
		{
			return handle < m_ParentIndices.size();
		}

		// --- Доступ к размерам и плоским SoA данным для подсистем (SpatialStorage, Renderer) ---
		[[nodiscard]] size_t GetNodeCount() const noexcept { return m_ParentIndices.size(); }
		[[nodiscard]] std::span<const Mat4<zF32>> GetWorldMatrices() const noexcept { return m_WorldMatrices; }
		[[nodiscard]] std::span<const NodeHandle> GetParentIndices() const noexcept { return m_ParentIndices; }
		[[nodiscard]] std::span<const zU32> GetSubtreeEnds() const noexcept { return m_SubtreeEnds; }
		[[nodiscard]] std::span<const eNodeFlags> GetFlags() const noexcept { return m_Flags; }
		[[nodiscard]] std::span<const NodeBindings> GetBindings() const noexcept { return m_Bindings; }
		[[nodiscard]] std::span<const Transform> GetLocalTransforms() const noexcept { return m_LocalTransforms; }
#pragma endregion

	private:
		void InitializeFromObjects(std::span<const zzz::core::GameObjectData> objects);

		std::vector<NodeHandle>		m_ParentIndices;
		std::vector<zU32>			m_SubtreeEnds;
		std::vector<Transform>		m_LocalTransforms;
		std::vector<Mat4<zF32>>		m_WorldMatrices;
		std::vector<eNodeFlags>		m_Flags;
		std::vector<NodeBindings>	m_Bindings;

		BitTreeTracker m_DirtyTracker;
		std::vector<TransformChangeRange> m_ChangeRanges;
	};
}
