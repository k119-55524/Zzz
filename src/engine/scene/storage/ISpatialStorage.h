#pragma once

#include <vector>
#include "math/utils/Types.h"

using namespace zzz::math;

namespace zzz::engine
{
	class NodeStorage;

	constexpr zU32 c_InvalidSpatialHandle = 0xFFFFFFFF;

	/**
	 * @class ISpatialStorage
	 * @brief Базовый интерфейс пространственного хранилища слоя.
	 *
	 * @details Реализует регистрацию и выборку пространственных элементов (userData).
	 * Точки расширения для последующих этапов (Этап 18+): AABB, QueryFrustum, Raycast.
	 */
	class ISpatialStorage
	{
	public:
		virtual ~ISpatialStorage() = default;

		/// @brief Пакетное построение пространственного индекса по плоскому списку узлов.
		virtual void Build(const NodeStorage& nodeStorage) = 0;
		virtual zU32 Insert(uint64_t userData) = 0;
		virtual void Remove(zU32 handle) = 0;
		virtual void Clear() = 0;

		virtual void GetAll(std::vector<uint64_t>& outUserData) const = 0;
		[[nodiscard]] virtual size_t GetCount() const noexcept = 0;
	};
}
