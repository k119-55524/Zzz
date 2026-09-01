#pragma once

#include <cstddef>
#include <iterator>

namespace zzz::math
{
	/**
	 * @class AttributeRange
	 * @brief Шаблонный диапазон доступа к атрибутам со страйдом (Zero-Copy).
	 *
	 * @details Позволяет обходить чередующиеся (interleaved) массивы данных без промежуточных аллокаций.
	 *          Если запрашиваемый атрибут отсутствует, создается пустой диапазон (count = 0),
	 *          и любой range-based for цикл автоматически безопасно пропускается.
	 *
	 * @tparam T Тип возвращаемого атрибута (например, Vec3f, Vec2f, Color4f).
	 */
	template<typename T>
	class AttributeRange
	{
		const std::byte* m_Start = nullptr;
		std::size_t      m_Stride = 0;
		std::size_t      m_Count = 0;

	public:
		constexpr AttributeRange() noexcept = default;

		constexpr AttributeRange(const std::byte* start, std::size_t stride, std::size_t count) noexcept
			: m_Start(start)
			, m_Stride(stride)
			, m_Count(count)
		{
		}

		/// @brief Доступ к элементу по индексу за O(1).
		[[nodiscard]] const T& operator[](std::size_t index) const noexcept
		{
			return *reinterpret_cast<const T*>(m_Start + (index * m_Stride));
		}

		[[nodiscard]] std::size_t size() const noexcept { return m_Count; }
		[[nodiscard]] bool empty() const noexcept { return m_Count == 0; }

		// --- Поддержка итераторов для range-based for ---

		struct Iterator
		{
			using iterator_category = std::forward_iterator_tag;
			using value_type        = T;
			using difference_type   = std::ptrdiff_t;
			using pointer           = const T*;
			using reference         = const T&;

			const std::byte* ptr = nullptr;
			std::size_t      stride = 0;

			[[nodiscard]] const T& operator*() const noexcept
			{
				return *reinterpret_cast<const T*>(ptr);
			}

			[[nodiscard]] const T* operator->() const noexcept
			{
				return reinterpret_cast<const T*>(ptr);
			}

			Iterator& operator++() noexcept
			{
				ptr += stride;
				return *this;
			}

			Iterator operator++(int) noexcept
			{
				Iterator tmp = *this;
				ptr += stride;
				return tmp;
			}

			[[nodiscard]] bool operator==(const Iterator& other) const noexcept
			{
				return ptr == other.ptr;
			}

			[[nodiscard]] bool operator!=(const Iterator& other) const noexcept
			{
				return ptr != other.ptr;
			}
		};

		[[nodiscard]] Iterator begin() const noexcept { return { m_Start, m_Stride }; }
		[[nodiscard]] Iterator end() const noexcept   { return { m_Start + (m_Count * m_Stride), m_Stride }; }
	};
}
