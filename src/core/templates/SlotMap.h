#pragma once

#include <vector>
#include <span>
#include <cstdint>
#include <concepts>
#include "core/utils/Defines.h"
#include "core/utils/Ensure.h"

namespace zzz::core
{
	/**
	 * @struct SlotHandle
	 * @brief Стабильный поколенческий идентификатор элемента в SlotMap.
	 */
	struct SlotHandle
	{
		uint32_t id{ 0 };         ///< 1-based индекс слота в разреженной таблице (0 = невалидный)
		uint32_t generation{ 0 }; ///< Поколение слота

		[[nodiscard]] constexpr bool IsValid() const noexcept { return id != 0; }
		[[nodiscard]] constexpr auto operator<=>(const SlotHandle&) const noexcept = default;
	};

	/**
	 * @class SlotMap
	 * @brief Высокопроизводительный разреженно-плотный контейнер (Dense/Sparse Pool).
	 *
	 * @details Хранит элементы непрерывно в векторе m_Dense (Swap & Pop при удалении).
	 * Разреженная таблица m_Sparse транслирует стабильный SlotHandle в текущий индекс m_Dense.
	 */
	template <typename T>
	class SlotMap final
	{
	public:
		SlotMap() = default;
		explicit SlotMap(size_t initialCapacity)
		{
			m_Dense.reserve(initialCapacity);
			m_DenseToSparse.reserve(initialCapacity);
			m_Sparse.reserve(initialCapacity);
		}

		~SlotMap() = default;

		SlotMap(const SlotMap&) = default;
		SlotMap& operator=(const SlotMap&) = default;
		SlotMap(SlotMap&&) noexcept = default;
		SlotMap& operator=(SlotMap&&) noexcept = default;

		/**
		 * @brief Добавляет элемент в контейнер.
		 * @return Стабильный поколенческий SlotHandle.
		 */
		template <typename... Args>
		SlotHandle Emplace(Args&&... args)
		{
			m_Dense.emplace_back(std::forward<Args>(args)...);
			const uint32_t denseIdx = static_cast<uint32_t>(m_Dense.size() - 1);

			uint32_t sparseIdx = 0;
			if (!m_FreeIndices.empty())
			{
				sparseIdx = m_FreeIndices.back();
				m_FreeIndices.pop_back();
				m_Sparse[sparseIdx].denseIndex = denseIdx;
			}
			else
			{
				sparseIdx = static_cast<uint32_t>(m_Sparse.size());
				m_Sparse.push_back(Slot{ .denseIndex = denseIdx, .generation = 1 });
			}

			m_DenseToSparse.push_back(sparseIdx);

			return SlotHandle{
				.id = sparseIdx + 1,
				.generation = m_Sparse[sparseIdx].generation
			};
		}

		/**
		 * @brief Удаляет элемент по хэндлу за O(1) через Swap & Pop.
		 * @return true, если элемент существовал и был удален.
		 */
		bool Remove(SlotHandle handle)
		{
			if (!IsAlive(handle))
			{
				return false;
			}

			const uint32_t sparseIdx = handle.id - 1;
			const uint32_t denseIdx = m_Sparse[sparseIdx].denseIndex;
			const uint32_t lastDenseIdx = static_cast<uint32_t>(m_Dense.size() - 1);

			if (denseIdx != lastDenseIdx)
			{
				// Swap & Pop в m_Dense
				m_Dense[denseIdx] = std::move(m_Dense[lastDenseIdx]);

				// Обновляем связь перемещенного элемента
				const uint32_t movedSparseIdx = m_DenseToSparse[lastDenseIdx];
				m_DenseToSparse[denseIdx] = movedSparseIdx;
				m_Sparse[movedSparseIdx].denseIndex = denseIdx;
			}

			m_Dense.pop_back();
			m_DenseToSparse.pop_back();

			// Инвалидируем поколение и добавляем слот в список свободных
			++m_Sparse[sparseIdx].generation;
			m_FreeIndices.push_back(sparseIdx);

			return true;
		}

		/**
		 * @brief Проверяет валидность и актуальность поколения хэндла.
		 */
		[[nodiscard]] bool IsAlive(SlotHandle handle) const noexcept
		{
			if (!handle.IsValid())
			{
				return false;
			}

			const uint32_t sparseIdx = handle.id - 1;
			if (sparseIdx >= m_Sparse.size())
			{
				return false;
			}

			return m_Sparse[sparseIdx].generation == handle.generation;
		}

		/**
		 * @brief Доступ к элементу по хэндлу.
		 */
		[[nodiscard]] T* Get(SlotHandle handle) noexcept
		{
			if (!IsAlive(handle))
			{
				return nullptr;
			}
			const uint32_t denseIdx = m_Sparse[handle.id - 1].denseIndex;
			return &m_Dense[denseIdx];
		}

		[[nodiscard]] const T* Get(SlotHandle handle) const noexcept
		{
			if (!IsAlive(handle))
			{
				return nullptr;
			}
			const uint32_t denseIdx = m_Sparse[handle.id - 1].denseIndex;
			return &m_Dense[denseIdx];
		}

		/**
		 * @brief Прямой доступ к непрерывному массиву элементов.
		 */
		[[nodiscard]] std::span<T> GetDenseSpan() noexcept { return m_Dense; }
		[[nodiscard]] std::span<const T> GetDenseSpan() const noexcept { return m_Dense; }

		[[nodiscard]] size_t Size() const noexcept { return m_Dense.size(); }
		[[nodiscard]] bool IsEmpty() const noexcept { return m_Dense.empty(); }

		void Clear() noexcept
		{
			m_Dense.clear();
			m_DenseToSparse.clear();
			m_Sparse.clear();
			m_FreeIndices.clear();
		}

	private:
		struct Slot
		{
			uint32_t denseIndex{ 0 };
			uint32_t generation{ 1 };
		};

		std::vector<T> m_Dense;
		std::vector<uint32_t> m_DenseToSparse;
		std::vector<Slot> m_Sparse;
		std::vector<uint32_t> m_FreeIndices;
	};
}
