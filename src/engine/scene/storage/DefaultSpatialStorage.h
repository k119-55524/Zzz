#pragma once

#include <span>
#include <vector>
#include "engine/scene/storage/ISpatialStorage.h"

namespace zzz::engine
{
	/**
	 * @class DefaultSpatialStorage
	 * @brief Линейная реализация пространственного хранилища для узлов с геометрией.
	 *
	 * @details Хранит плотный непрерывный массив NodeHandle для узлов с геометрией (HasMesh),
	 * обеспечивая возврат std::span<const NodeHandle> без аллокаций.
	 */
	class DefaultSpatialStorage final : public ISpatialStorage
	{
	public:
		DefaultSpatialStorage() = default;
		~DefaultSpatialStorage() override = default;

		void Clear() override;
		SpatialHandle AddMeshNode(NodeHandle nodeHandle) override;

		[[nodiscard]] std::span<const NodeHandle> GetMeshNodes() const noexcept override
		{
			return m_MeshNodes;
		}

		[[nodiscard]] size_t GetCount() const noexcept override { return m_MeshNodes.size(); }

	private:
		std::vector<NodeHandle> m_MeshNodes;
	};
}
