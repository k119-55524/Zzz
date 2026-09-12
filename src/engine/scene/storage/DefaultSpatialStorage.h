#pragma once

#include <vector>
#include <cstdint>

#include "engine/scene/storage/ISpatialStorage.h"

namespace zzz::engine
{
	/**
	 * @class DefaultSpatialStorage
	 * @brief Простая линейная реализация пространственного хранилища (массив слотов).
	 *
	 * @details Хранит элементы в плоском массиве (std::vector) с повторным использованием
	 * освобождённых индексов через Free-List. Предоставляет доступ за O(1) и линейный GetAll.
	 */
	class DefaultSpatialStorage final : public ISpatialStorage
	{
	public:
		DefaultSpatialStorage() = default;
		~DefaultSpatialStorage() override = default;

		void Build(const NodeStorage& nodeStorage) override;
		SpatialHandle Insert(uint64_t userData) override;
		void Remove(SpatialHandle handle) override;
		void Clear() override;

		void GetAll(std::vector<uint64_t>& outUserData) const override;

		[[nodiscard]] size_t GetCount() const noexcept override { return m_ActiveCount; }

	private:
		struct Slot
		{
			uint64_t userData{ 0 };
			bool     isOccupied{ false };
		};

		std::vector<Slot>     m_Slots;
		std::vector<uint32_t> m_FreeIndices;
		size_t                m_ActiveCount{ 0 };
	};
}
