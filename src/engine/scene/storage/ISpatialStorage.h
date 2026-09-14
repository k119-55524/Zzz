#pragma once
 
#include <span>
#include <vector>
#include "math/utils/Types.h"
#include "engine/scene/storage/NodeTypes.h"

using namespace zzz::math;

namespace zzz::engine
{
	class NodeStorage;

	/**
	 * @class ISpatialStorage
	 * @brief Базовый интерфейс пространственного хранилища слоя.
	 *
	 * @details Регистрирует узлы с геометрией (HasMesh() == true) и возвращает
	 * плоский непрерывный диапазон NodeHandle для рендерера и подсистем отсечения.
	 */
	class ISpatialStorage
	{
	public:
		virtual ~ISpatialStorage() = default;

		/// @brief Очистка хранилища.
		virtual void Clear() = 0;

		/// @brief Регистрация узла с мешем в пространственном хранилище.
		virtual SpatialHandle AddMeshNode(NodeHandle nodeHandle) = 0;

		/// @brief Доступ к плоскому списку узлов, содержащих геометрию (HasMesh).
		[[nodiscard]] virtual std::span<const NodeHandle> GetMeshNodes() const noexcept = 0;

		[[nodiscard]] virtual size_t GetCount() const noexcept = 0;
	};
}
