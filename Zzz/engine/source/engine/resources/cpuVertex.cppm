#include "pch.h"
export module cpuVertex;

namespace zzz
{
	// ----- Универсальные семантики -----
	export enum class Semantic
	{
		Position,
		Normal,
		TexCoord,
		Color,
		Tangent,
		Bitangent
	};

	// ----- Универсальные форматы -----
	export enum class Format
	{
		Float1,
		Float2,
		Float3,
		Float4,
		UInt1,
		UInt2,
		UInt3,
		UInt4
	};

	// ----- Атрибут вершины -----
	export template<Semantic S, typename T, size_t N>
		struct Attribute
	{
		static_assert(N >= 1 && N <= 4,
			">>>>> [struct Attribute]. СomponentCount (N) must be in range [1..4]");
		static_assert(std::is_arithmetic_v<T>,
			">>>>> [struct Attribute]. Сomponent type T must be arithmetic (float, int, uint...)");

		using type = T;
		static constexpr Semantic semantic = S;
		static constexpr size_t componentCount = N;
		T value[N];

		Attribute() = default;

		// Конструктор для удобства
		Attribute(std::initializer_list<T> init)
		{
			assert(init.size() == N);
			std::copy(init.begin(), init.end(), value);
		}
	};

	export struct LayoutEntry
	{
		Semantic semantic;
		Format format;
		size_t offset;
	};

	// ----- Вершина -----
	export template<typename... Attrs>
		struct cpuVertex : public Attrs...
	{
		// Удаляем локальное определение LayoutEntry
		// Используем zzz::LayoutEntry

		using Attributes = std::tuple<Attrs...>;

		template<typename A>
		A& get() { return static_cast<A&>(*this); }

		template<typename A>
		const A& get() const { return static_cast<const A&>(*this); }

		template<typename A>
		static constexpr size_t offsetOf()
		{
			return reinterpret_cast<size_t>(
				&(reinterpret_cast<const A*>(
					reinterpret_cast<const cpuVertex*>(0)
					)->value)
				);
		}

		static constexpr auto layout()
		{
			std::array<LayoutEntry, sizeof...(Attrs)> arr{ {
				{ Attrs::semantic, deduceFormat<Attrs>(), offsetOf<Attrs>() }...
			} };
			return arr;
		}

		template<typename Attr>
		static constexpr Format deduceFormat()
		{
			if constexpr (std::is_same_v<typename Attr::type, float>)
			{
				if constexpr (Attr::componentCount == 1) return Format::Float1;
				if constexpr (Attr::componentCount == 2) return Format::Float2;
				if constexpr (Attr::componentCount == 3) return Format::Float3;
				if constexpr (Attr::componentCount == 4) return Format::Float4;
			}
			else
			{
				static_assert(false, "Unsupported attribute type or component count");
			}
			return Format::Float1; // Для подавления предупреждений о возврате
		}
	};

	// ----- Интерфейс для универсального буфера -----
	export struct ICPUVertexBuffer
	{
		virtual ~ICPUVertexBuffer() = default;

		virtual size_t stride() const = 0;
		virtual size_t sizeInBytes() const = 0;
		virtual const void* rawData() const = 0;
		virtual size_t vertexCount() const = 0;

		// Удаляем локальное определение LayoutEntry
		virtual std::vector<LayoutEntry> layout() const = 0;
	};

	// ----- Шаблон буфера вершин -----
	export template<typename... Attrs>
		struct VertexBuffer : public ICPUVertexBuffer
	{
		using VertexT = cpuVertex<Attrs...>;
		std::vector<VertexT> vertices;

		VertexBuffer() = default;
		VertexBuffer(std::initializer_list<VertexT> init) : vertices(init) {}
		~VertexBuffer() = default;

		VertexBuffer(VertexBuffer&&) noexcept = default;
		VertexBuffer& operator=(VertexBuffer&&) noexcept = default;

		VertexBuffer(const VertexBuffer&) = delete;
		VertexBuffer& operator=(const VertexBuffer&) = delete;

		size_t stride() const override { return sizeof(VertexT); }
		size_t sizeInBytes() const override { return vertices.size() * sizeof(VertexT); }
		size_t vertexCount() const override { return vertices.size(); }
		const void* rawData() const override { return vertices.data(); }

		std::vector<LayoutEntry> layout() const override
		{
			auto arr = VertexT::layout();
			return { arr.begin(), arr.end() };
		}
	};

	// ----- Готовые типы -----
	export using VB_P3N3 = VertexBuffer<
		Attribute<Semantic::Position, float, 3>,
		Attribute<Semantic::Normal, float, 3>>;

	export using VB_P3C3 = VertexBuffer<
		Attribute<Semantic::Position, float, 3>,
		Attribute<Semantic::Color, float, 3>>;

	export using VB_P3N3T3 = VertexBuffer<
		Attribute<Semantic::Position, float, 3>,
		Attribute<Semantic::Normal, float, 3>,
		Attribute<Semantic::TexCoord, float, 2>>;
}

namespace zzz
{
	// Универсальный интерфейс для маппинга форматов/семантик под конкретное API
	struct IVertexFormatMapper
	{
		virtual ~IVertexFormatMapper() = default;

		// Вернуть API-специфичный enum для формата
		virtual int mapFormat(Format f) const = 0;

		// Вернуть API-специфичный индекс/идентификатор для семантики
		virtual int mapSemantic(Semantic s) const = 0;
	};
}

namespace zzz
{
#if defined(_WIN64)
	struct DX12FormatMapper final : public IVertexFormatMapper
	{
		int mapFormat(Format f) const override
		{
			switch (f)
			{
			case Format::Float1: return DXGI_FORMAT_R32_FLOAT;
			case Format::Float2: return DXGI_FORMAT_R32G32_FLOAT;
			case Format::Float3: return DXGI_FORMAT_R32G32B32_FLOAT;
			case Format::Float4: return DXGI_FORMAT_R32G32B32A32_FLOAT;

			case Format::UInt1:  return DXGI_FORMAT_R32_UINT;
			case Format::UInt2:  return DXGI_FORMAT_R32G32_UINT;
			case Format::UInt3:  return DXGI_FORMAT_R32G32B32_UINT;
			case Format::UInt4:  return DXGI_FORMAT_R32G32B32A32_UINT;

			default:             return DXGI_FORMAT_UNKNOWN;
			}
		}

		int mapSemantic(Semantic s) const override
		{
			// В D3D12 семантика задаётся строкой (POSITION, NORMAL...),
			// но можно использовать индекс для генерации строк
			return static_cast<int>(s);
		}
	};
#endif
}

namespace zzz
{
	//struct VulkanFormatMapper final : public IVertexFormatMapper
	//{
	//	int mapFormat(Format f) const override
	//	{
	//		switch (f)
	//		{
	//		case Format::Float1: return VK_FORMAT_R32_SFLOAT;
	//		case Format::Float2: return VK_FORMAT_R32G32_SFLOAT;
	//		case Format::Float3: return VK_FORMAT_R32G32B32_SFLOAT;
	//		case Format::Float4: return VK_FORMAT_R32G32B32A32_SFLOAT;

	//		case Format::UInt1:  return VK_FORMAT_R32_UINT;
	//		case Format::UInt2:  return VK_FORMAT_R32G32_UINT;
	//		case Format::UInt3:  return VK_FORMAT_R32G32B32_UINT;
	//		case Format::UInt4:  return VK_FORMAT_R32G32B32A32_UINT;

	//		default:             return VK_FORMAT_UNDEFINED;
	//		}
	//	}

	//	int mapSemantic(Semantic s) const override
	//	{
	//		// В Vulkan нет фиксированных семантик -> используем location
	//		return static_cast<int>(s);
	//	}
	//};
}

//namespace zzz
//{
//	struct MetalFormatMapper final : public IVertexFormatMapper
//	{
//		int mapFormat(Format f) const override
//		{
//			switch (f)
//			{
//			case Format::Float1: return MTLVertexFormatFloat;
//			case Format::Float2: return MTLVertexFormatFloat2;
//			case Format::Float3: return MTLVertexFormatFloat3;
//			case Format::Float4: return MTLVertexFormatFloat4;
//
//			case Format::UInt1:  return MTLVertexFormatUInt;
//			case Format::UInt2:  return MTLVertexFormatUInt2;
//			case Format::UInt3:  return MTLVertexFormatUInt3;
//			case Format::UInt4:  return MTLVertexFormatUInt4;
//
//			default:             return MTLVertexFormatInvalid;
//			}
//		}
//
//		int mapSemantic(Semantic s) const override
//		{
//			// В Metal семантика мапится на "attribute index"
//			switch (s)
//			{
//			case Semantic::Position:  return 0;
//			case Semantic::Normal:    return 1;
//			case Semantic::TexCoord:  return 2;
//			case Semantic::Color:     return 3;
//			case Semantic::Tangent:   return 4;
//			case Semantic::Bitangent: return 5;
//			}
//			return -1;
//		}
//	};
//}