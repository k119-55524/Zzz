#pragma once

#include "core/CoreIncludes.h"
#include <array>

namespace zzz::core
{
	/**
	 * @brief Двойной буфер для кадрового цикла (Prepare / Render).
	 * Индекс Prepare обновляет наполняемые данные кадра,
	 * Индекс Render используется потоком рендеринга.
	 */
	template <typename T>
	class SwapQueue final
	{
	public:
		SwapQueue() : m_IndexPrepare(0), m_IndexRender(1) {}
		explicit SwapQueue(T initialData) : m_IndexPrepare(0), m_IndexRender(1)
		{
			m_Buffers[0] = initialData;
			m_Buffers[1] = initialData;
		}

		[[nodiscard]] inline T& GetPrepareBuffer() noexcept { return m_Buffers[m_IndexPrepare]; }
		[[nodiscard]] inline const T& GetPrepareBuffer() const noexcept { return m_Buffers[m_IndexPrepare]; }

		[[nodiscard]] inline T& GetRenderBuffer() noexcept { return m_Buffers[m_IndexRender]; }
		[[nodiscard]] inline const T& GetRenderBuffer() const noexcept { return m_Buffers[m_IndexRender]; }

		[[nodiscard]] inline uint32_t GetPrepareIndex() const noexcept { return m_IndexPrepare; }
		[[nodiscard]] inline uint32_t GetRenderIndex() const noexcept { return m_IndexRender; }

		inline void Swap() noexcept
		{
			std::swap(m_IndexPrepare, m_IndexRender);
		}

	private:
		std::array<T, 2> m_Buffers{};
		uint32_t m_IndexPrepare{ 0 };
		uint32_t m_IndexRender{ 1 };
	};
}
