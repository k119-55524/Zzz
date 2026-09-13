
#include <algorithm>
#include "core/io/package/GameObjectData.h"

#include "NodeStorage.h"

using namespace zzz::core;
using namespace zzz::math;

namespace zzz::engine
{
	NodeStorage::NodeStorage()
		: m_ParentIndices()
		, m_SubtreeEnds()
		, m_LocalTransforms()
		, m_WorldMatrices()
		, m_Flags()
		, m_Bindings()
		, m_NodeVisuals()
		, m_Draws()
		, m_DirtyTracker(1)
		, m_ChangeRanges()
	{
	}

	NodeStorage::NodeStorage(std::span<const zzz::core::GameObjectData> objects)
		: m_ParentIndices()
		, m_SubtreeEnds()
		, m_LocalTransforms()
		, m_WorldMatrices()
		, m_Flags()
		, m_Bindings()
		, m_NodeVisuals()
		, m_Draws()
		, m_DirtyTracker(static_cast<zU32>(objects.empty() ? 1 : objects.size()))
		, m_ChangeRanges()
	{
		InitializeFromObjects(objects);
	}

	void NodeStorage::InitializeFromObjects(std::span<const zzz::core::GameObjectData> objects)
	{
		const size_t count = objects.size();
		if (count == 0)
			return;

		ensure(count < kInvalidNodeHandle, "NodeStorage: количество объектов ({}) превышает максимально допустимую емкость kInvalidNodeHandle", count);

		m_ParentIndices.resize(count);
		m_SubtreeEnds.resize(count);
		m_LocalTransforms.resize(count);
		m_WorldMatrices.resize(count);
		m_Flags.resize(count);
		m_Bindings.resize(count);
		m_NodeVisuals.resize(count);
		m_ChangeRanges.reserve(count);

		// Предварительный подсчет draw calls для исключения реаллокаций m_Draws
		size_t totalDraws = 0;
		for (size_t i = 0; i < count; ++i)
		{
			const auto& obj = objects[i];
			if (obj.IsMultiMesh())
			{
				totalDraws += obj.GetSubmeshGuids().size();
			}
			else if (obj.GetMeshGuid().IsValid())
			{
				totalDraws += 1;
			}
		}
		ensure(totalDraws <= static_cast<size_t>(std::numeric_limits<zU32>::max()),
			"NodeStorage: суммарное количество draw calls ({}) превышает вместимость zU32", totalDraws);
		m_Draws.reserve(totalDraws);

		m_DirtyTracker.Prepare(static_cast<zU32>(count));

		// 1. Заполняем плоские массивы узлов и проверяем инвариант parentIndex < i
		for (size_t i = 0; i < count; ++i)
		{
			const auto& obj = objects[i];
			const zU32 pIdx = obj.GetParentIndex();

			if (pIdx == 0xFFFFFFFF)
			{
				m_ParentIndices[i] = kInvalidNodeHandle;
			}
			else
			{
				ensure(pIdx < i, "NodeStorage: нарушение инварианта parentIndex < childIndex (parent: {}, child: {})", pIdx, i);
				m_ParentIndices[i] = static_cast<NodeHandle>(pIdx);
			}

			m_SubtreeEnds[i] = static_cast<zU32>(i + 1);

			m_LocalTransforms[i].position = obj.GetPosition();
			m_LocalTransforms[i].rotation = obj.GetRotation();
			m_LocalTransforms[i].scale = obj.GetScale();

			m_Flags[i] = (obj.IsActive() ? eNodeFlags::Active : eNodeFlags::None) | eNodeFlags::Visible;

			m_Bindings[i] = NodeBindings{
				.domainHandle = kInvalidDomainHandle,
				.spatialHandle = kInvalidSpatialHandle,
				.domainKind = eNodeDomainKind::None
			};

			if (obj.IsMultiMesh())
			{
				const zU32 begin = static_cast<zU32>(m_Draws.size());
				const auto& submeshes = obj.GetSubmeshGuids();
				const auto& materials = obj.GetMaterialGuids();
				const size_t submeshCount = submeshes.size();
				for (size_t s = 0; s < submeshCount; ++s)
				{
					const Guid matGuid = (s < materials.size()) ? materials[s] : Guid();
					m_Draws.push_back(DrawDescriptor{
						.meshGuid = submeshes[s],
						.materialGuid = matGuid
					});
				}
				m_NodeVisuals[i] = VisualRange{
					.begin = begin,
					.count = static_cast<zU32>(submeshCount)
				};
			}
			else if (obj.GetMeshGuid().IsValid())
			{
				const zU32 begin = static_cast<zU32>(m_Draws.size());
				m_Draws.push_back(DrawDescriptor{
					.meshGuid = obj.GetMeshGuid(),
					.materialGuid = obj.GetMaterialGuid()
				});
				m_NodeVisuals[i] = VisualRange{
					.begin = begin,
					.count = 1
				};
			}
			else
			{
				m_NodeVisuals[i] = VisualRange{
					.begin = 0,
					.count = 0
				};
			}
		}

		// 2. Расчет subtreeEnd в один обратный проход от листьев к корням
		for (size_t i = count; i > 0; --i)
		{
			const size_t idx = i - 1;
			const NodeHandle pIdx = m_ParentIndices[idx];
			if (pIdx != kInvalidNodeHandle && pIdx < count)
			{
				m_SubtreeEnds[pIdx] = std::max(m_SubtreeEnds[pIdx], m_SubtreeEnds[idx]);
			}
		}

		// 3. Вычисление мировых матриц в один прямой проход по preorder-массиву
		for (size_t i = 0; i < count; ++i)
		{
			const auto& local = m_LocalTransforms[i];
			const Mat4<zF32> localMatrix = Mat4<zF32>::Scaling(local.scale) * local.rotation.ToMat4() * Mat4<zF32>::Translation(local.position);

			const NodeHandle pIdx = m_ParentIndices[i];
			if (pIdx != kInvalidNodeHandle && pIdx < count)
			{
				m_WorldMatrices[i] = localMatrix * m_WorldMatrices[pIdx];
			}
			else
			{
				m_WorldMatrices[i] = localMatrix;
			}
		}
	}

	std::span<const TransformChangeRange> NodeStorage::ResolveTransforms()
	{
		m_ChangeRanges.clear();

		const auto dirtyIndices = m_DirtyTracker.ConsumeDirtyIndices();
		if (dirtyIndices.empty())
		{
			return m_ChangeRanges;
		}

		const size_t count = m_ParentIndices.size();
		zU32 coveredUntil = 0;

		for (const zU32 nodeIndex : dirtyIndices)
		{
			if (nodeIndex >= count)
			{
				continue;
			}

			// Пропускаем узлы, уже обработанные в составе более высокого грязного поддерева
			if (nodeIndex < coveredUntil)
			{
				continue;
			}

			const zU32 rangeBegin = nodeIndex;
			const zU32 rangeEnd = m_SubtreeEnds[nodeIndex];
			coveredUntil = rangeEnd;

			// Линейный пересчет непрерывного поддерева [rangeBegin, rangeEnd) без проверки isActive
			for (zU32 i = rangeBegin; i < rangeEnd; ++i)
			{
				const auto& local = m_LocalTransforms[i];
				const Mat4<zF32> localMatrix = Mat4<zF32>::Scaling(local.scale) * local.rotation.ToMat4() * Mat4<zF32>::Translation(local.position);

				const NodeHandle pIdx = m_ParentIndices[i];
				if (pIdx != kInvalidNodeHandle && pIdx < count)
				{
					m_WorldMatrices[i] = localMatrix * m_WorldMatrices[pIdx];
				}
				else
				{
					m_WorldMatrices[i] = localMatrix;
				}
			}

			// Объединение смежных диапазонов
			if (!m_ChangeRanges.empty() && m_ChangeRanges.back().end == rangeBegin)
			{
				m_ChangeRanges.back().end = rangeEnd;
			}
			else
			{
				m_ChangeRanges.push_back(TransformChangeRange{
					.begin = rangeBegin,
					.end = rangeEnd
				});
			}
		}

		return m_ChangeRanges;
	}
}

