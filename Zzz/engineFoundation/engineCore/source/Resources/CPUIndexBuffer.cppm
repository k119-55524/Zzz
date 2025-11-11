
#include "pch.h"

export module CPUIndexBuffer;

namespace zzz::engineCore
{
	// ===== Интерфейс индексного буфера =====
	export struct ICPUIndexBuffer
	{
		virtual ~ICPUIndexBuffer() = default;

		// Получение размера буфера в байтах
		virtual size_t GetSizeInBytes() const = 0;

		// Количество индексов
		virtual size_t GetCount() const = 0;

		// Указатель на данные (тип неизвестен, поэтому const void*)
		virtual const void* GetData() const = 0;

#if defined(_WIN64)
		// DXGI формат
		virtual DXGI_FORMAT GetFormat() const = 0;
#endif
	};

	// ===== Шаблонная реализация =====
	template <typename T>
	class CPUIndexBuffer final : public ICPUIndexBuffer
	{
	public:
		using IndexType = T; // uint16_t или uint32_t

		explicit CPUIndexBuffer(const std::vector<T>& indices) :
			m_Indices(indices)
		{
			static_assert(std::is_same_v<T, uint16_t> || std::is_same_v<T, uint32_t>,
				"IndexBuffer поддерживает только uint16_t или uint32_t");
		}

		explicit CPUIndexBuffer(std::initializer_list<T> indices) :
			m_Indices(indices)
		{
			static_assert(std::is_same_v<T, uint16_t> || std::is_same_v<T, uint32_t>,
				"IndexBuffer поддерживает только uint16_t или uint32_t");
		}

		size_t GetSizeInBytes() const override { return m_Indices.size() * sizeof(T); }
		size_t GetCount() const override { return m_Indices.size(); }
		const void* GetData() const override { return m_Indices.data(); }

#if defined(_WIN64)
		DXGI_FORMAT GetFormat() const override
		{
			if constexpr (std::is_same_v<T, uint16_t>)
				return DXGI_FORMAT_R16_UINT;
			else if constexpr (std::is_same_v<T, uint32_t>)
				return DXGI_FORMAT_R32_UINT;
			else
				throw std::runtime_error("Неподдерживаемый тип индекса");
		}
#endif

		// Дополнительно — доступ к исходному вектору в типе T
		const std::vector<T>& GetIndices() const { return m_Indices; }

	private:
		std::vector<T> m_Indices;
	};

	// Удобные псевдонимы
	export using IndexBuffer16 = CPUIndexBuffer<uint16_t>;
	export using IndexBuffer32 = CPUIndexBuffer<uint32_t>;
}
