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
	export enum class VertexFormat
	{
		Unknown,

		// 8-bit
		R8_UNorm, R8_SNorm, R8_UInt, R8_SInt,
		RG8_UNorm, RG8_SNorm, RG8_UInt, RG8_SInt,
		RGBA8_UNorm, RGBA8_SNorm, RGBA8_UInt, RGBA8_SInt,
		RGBA8_UNorm_sRGB, // sRGB гамма

		// 16-bit
		R16_UNorm, R16_SNorm, R16_UInt, R16_SInt, R16_Float,
		RG16_UNorm, RG16_SNorm, RG16_UInt, RG16_SInt, RG16_Float,
		RGBA16_UNorm, RGBA16_SNorm, RGBA16_UInt, RGBA16_SInt, RGBA16_Float,

		// 32-bit
		R32_UInt, R32_SInt, R32_Float,
		RG32_UInt, RG32_SInt, RG32_Float,
		RGB32_UInt, RGB32_SInt, RGB32_Float,
		RGBA32_UInt, RGBA32_SInt, RGBA32_Float,

		// Special / HDR
		RGB10A2_UNorm, RGB10A2_UInt,
		R11G11B10_Float,
		R9G9B9E5_SharedExp
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
		VertexFormat format;
		size_t offset;
	};

	// ----- Вершина -----
	export template<typename... Attrs>
		struct cpuVertex : public Attrs...
	{
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
		static constexpr VertexFormat deduceFormat()
		{
			using T = typename Attr::type;
			constexpr size_t N = Attr::componentCount;

			// ----- float -----
			if constexpr (std::is_same_v<T, float>)
			{
				if constexpr (N == 1) return VertexFormat::R32_Float;
				if constexpr (N == 2) return VertexFormat::RG32_Float;
				if constexpr (N == 3) return VertexFormat::RGB32_Float;
				if constexpr (N == 4) return VertexFormat::RGBA32_Float;
			}
			// ----- unsigned int -----
			else if constexpr (std::is_same_v<T, uint32_t>)
			{
				if constexpr (N == 1) return VertexFormat::R32_UInt;
				if constexpr (N == 2) return VertexFormat::RG32_UInt;
				if constexpr (N == 3) return VertexFormat::RGB32_UInt;
				if constexpr (N == 4) return VertexFormat::RGBA32_UInt;
			}
			// ----- signed int -----
			else if constexpr (std::is_same_v<T, int32_t>)
			{
				if constexpr (N == 1) return VertexFormat::R32_SInt;
				if constexpr (N == 2) return VertexFormat::RG32_SInt;
				if constexpr (N == 3) return VertexFormat::RGB32_SInt;
				if constexpr (N == 4) return VertexFormat::RGBA32_SInt;
			}
			// ----- uint8_t -----
			else if constexpr (std::is_same_v<T, uint8_t>)
			{
				if constexpr (N == 4) return VertexFormat::RGBA8_UNorm;
			}
			// ----- int8_t -----
			else if constexpr (std::is_same_v<T, int8_t>)
			{
				if constexpr (N == 4) return VertexFormat::RGBA8_SNorm;
			}

#ifdef __APPLE__
			// ----- float16 (Metal only) -----
			else if constexpr (std::is_same_v<T, half>)
			{
				if constexpr (N == 1) return VertexFormat::R16_Float;
				if constexpr (N == 2) return VertexFormat::RG16_Float;
				if constexpr (N == 3) return VertexFormat::RGBA16_Float; // нет RGB16_Float
				if constexpr (N == 4) return VertexFormat::RGBA16_Float;
			}
#endif

			else
			{
				static_assert(!sizeof(T), "Unsupported attribute type in deduceFormat");
			}

			return VertexFormat::Unknown;
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
	export struct IVertexFormatMapper
	{
		virtual ~IVertexFormatMapper() = default;

		// Вернуть API-специфичный enum/код для формата
		virtual int mapFormat(VertexFormat f) const = 0;

		// Вернуть API-специфичный индекс/идентификатор для семантики
		virtual int mapSemanticIndex(Semantic s) const = 0;
	};

#if defined(_WIN64)
	// =====================================================================
	// ---------------------- DirectX 12 маппер ----------------------------
	// =====================================================================
	export struct DXVertexFormatMapper final : IVertexFormatMapper
	{
	private:
		struct Mapping { VertexFormat vf; DXGI_FORMAT dx; };
		static constexpr Mapping mappings[] = {
			// 8-bit
			{VertexFormat::R8_UNorm, DXGI_FORMAT_R8_UNORM},
			{VertexFormat::R8_SNorm, DXGI_FORMAT_R8_SNORM},
			{VertexFormat::R8_UInt,  DXGI_FORMAT_R8_UINT},
			{VertexFormat::R8_SInt,  DXGI_FORMAT_R8_SINT},

			{VertexFormat::RG8_UNorm, DXGI_FORMAT_R8G8_UNORM},
			{VertexFormat::RG8_SNorm, DXGI_FORMAT_R8G8_SNORM},
			{VertexFormat::RG8_UInt,  DXGI_FORMAT_R8G8_UINT},
			{VertexFormat::RG8_SInt,  DXGI_FORMAT_R8G8_SINT},

			{VertexFormat::RGBA8_UNorm, DXGI_FORMAT_R8G8B8A8_UNORM},
			{VertexFormat::RGBA8_SNorm, DXGI_FORMAT_R8G8B8A8_SNORM},
			{VertexFormat::RGBA8_UInt,  DXGI_FORMAT_R8G8B8A8_UINT},
			{VertexFormat::RGBA8_SInt,  DXGI_FORMAT_R8G8B8A8_SINT},
			{VertexFormat::RGBA8_UNorm_sRGB, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB},

			// 16-bit
			{VertexFormat::R16_UNorm, DXGI_FORMAT_R16_UNORM},
			{VertexFormat::R16_SNorm, DXGI_FORMAT_R16_SNORM},
			{VertexFormat::R16_UInt,  DXGI_FORMAT_R16_UINT},
			{VertexFormat::R16_SInt,  DXGI_FORMAT_R16_SINT},
			{VertexFormat::R16_Float, DXGI_FORMAT_R16_FLOAT},

			{VertexFormat::RG16_UNorm, DXGI_FORMAT_R16G16_UNORM},
			{VertexFormat::RG16_SNorm, DXGI_FORMAT_R16G16_SNORM},
			{VertexFormat::RG16_UInt,  DXGI_FORMAT_R16G16_UINT},
			{VertexFormat::RG16_SInt,  DXGI_FORMAT_R16G16_SINT},
			{VertexFormat::RG16_Float, DXGI_FORMAT_R16G16_FLOAT},

			{VertexFormat::RGBA16_UNorm, DXGI_FORMAT_R16G16B16A16_UNORM},
			{VertexFormat::RGBA16_SNorm, DXGI_FORMAT_R16G16B16A16_SNORM},
			{VertexFormat::RGBA16_UInt,  DXGI_FORMAT_R16G16B16A16_UINT},
			{VertexFormat::RGBA16_SInt,  DXGI_FORMAT_R16G16B16A16_SINT},
			{VertexFormat::RGBA16_Float, DXGI_FORMAT_R16G16B16A16_FLOAT},

			// 32-bit
			{VertexFormat::R32_UInt, DXGI_FORMAT_R32_UINT},
			{VertexFormat::R32_SInt, DXGI_FORMAT_R32_SINT},
			{VertexFormat::R32_Float, DXGI_FORMAT_R32_FLOAT},

			{VertexFormat::RG32_UInt, DXGI_FORMAT_R32G32_UINT},
			{VertexFormat::RG32_SInt, DXGI_FORMAT_R32G32_SINT},
			{VertexFormat::RG32_Float, DXGI_FORMAT_R32G32_FLOAT},

			{VertexFormat::RGB32_UInt, DXGI_FORMAT_R32G32B32_UINT},
			{VertexFormat::RGB32_SInt, DXGI_FORMAT_R32G32B32_SINT},
			{VertexFormat::RGB32_Float, DXGI_FORMAT_R32G32B32_FLOAT},

			{VertexFormat::RGBA32_UInt, DXGI_FORMAT_R32G32B32A32_UINT},
			{VertexFormat::RGBA32_SInt, DXGI_FORMAT_R32G32B32A32_SINT},
			{VertexFormat::RGBA32_Float, DXGI_FORMAT_R32G32B32A32_FLOAT},

			// Packed / Special
			{VertexFormat::RGB10A2_UNorm, DXGI_FORMAT_R10G10B10A2_UNORM},
			{VertexFormat::RGB10A2_UInt, DXGI_FORMAT_R10G10B10A2_UINT},
			{VertexFormat::R11G11B10_Float, DXGI_FORMAT_R11G11B10_FLOAT},
			{VertexFormat::R9G9B9E5_SharedExp, DXGI_FORMAT_R9G9B9E5_SHAREDEXP},
		};

	public:
		static constexpr DXGI_FORMAT toDXGI(VertexFormat fmt) noexcept
		{
			for (auto m : mappings)
				if (m.vf == fmt) return m.dx;

			return DXGI_FORMAT_UNKNOWN;
		}

		int mapSemanticIndex(Semantic s) const override
		{
			switch (s)
			{
			case Semantic::Position: return 0;
			case Semantic::Normal:   return 1;
			case Semantic::TexCoord: return 2;
			case Semantic::Color:    return 3;
			case Semantic::Tangent:  return 4;
			case Semantic::Bitangent:return 5;
			default: throw std::runtime_error("DX: Unsupported semantic");
			}
		}
	};
#endif // defined(_WIN64)

#if defined(__ANDROID__) || defined(__linux__) //|| defined(_WIN64)
	// =====================================================================
	// ---------------------- Vulkan маппер --------------------------------
	// =====================================================================
	export struct VkVertexFormatMapper final : IVertexFormatMapper
	{
	private:
		struct Mapping { VertexFormat vf; VkFormat vk; };
		static constexpr Mapping mappings[] = {
			// 8-bit
			{VertexFormat::R8_UNorm, VK_FORMAT_R8_UNORM},
			{VertexFormat::R8_SNorm, VK_FORMAT_R8_SNORM},
			{VertexFormat::R8_UInt,  VK_FORMAT_R8_UINT},
			{VertexFormat::R8_SInt,  VK_FORMAT_R8_SINT},

			{VertexFormat::RG8_UNorm, VK_FORMAT_R8G8_UNORM},
			{VertexFormat::RG8_SNorm, VK_FORMAT_R8G8_SNORM},
			{VertexFormat::RG8_UInt,  VK_FORMAT_R8G8_UINT},
			{VertexFormat::RG8_SInt,  VK_FORMAT_R8G8_SINT},

			{VertexFormat::RGBA8_UNorm, VK_FORMAT_R8G8B8A8_UNORM},
			{VertexFormat::RGBA8_SNorm, VK_FORMAT_R8G8B8A8_SNORM},
			{VertexFormat::RGBA8_UInt,  VK_FORMAT_R8G8B8A8_UINT},
			{VertexFormat::RGBA8_SInt,  VK_FORMAT_R8G8B8A8_SINT},
			{VertexFormat::RGBA8_UNorm_sRGB, VK_FORMAT_R8G8B8A8_SRGB},

			// 16-bit
			{VertexFormat::R16_UNorm, VK_FORMAT_R16_UNORM},
			{VertexFormat::R16_SNorm, VK_FORMAT_R16_SNORM},
			{VertexFormat::R16_UInt,  VK_FORMAT_R16_UINT},
			{VertexFormat::R16_SInt,  VK_FORMAT_R16_SINT},
			{VertexFormat::R16_Float, VK_FORMAT_R16_SFLOAT},

			{VertexFormat::RG16_UNorm, VK_FORMAT_R16G16_UNORM},
			{VertexFormat::RG16_SNorm, VK_FORMAT_R16G16_SNORM},
			{VertexFormat::RG16_UInt,  VK_FORMAT_R16G16_UINT},
			{VertexFormat::RG16_SInt,  VK_FORMAT_R16G16_SINT},
			{VertexFormat::RG16_Float, VK_FORMAT_R16G16_SFLOAT},

			{VertexFormat::RGBA16_UNorm, VK_FORMAT_R16G16B16A16_UNORM},
			{VertexFormat::RGBA16_SNorm, VK_FORMAT_R16G16B16A16_SNORM},
			{VertexFormat::RGBA16_UInt,  VK_FORMAT_R16G16B16A16_UINT},
			{VertexFormat::RGBA16_SInt,  VK_FORMAT_R16G16B16A16_SINT},
			{VertexFormat::RGBA16_Float, VK_FORMAT_R16G16B16A16_SFLOAT},

			// 32-bit
			{VertexFormat::R32_UInt, VK_FORMAT_R32_UINT},
			{VertexFormat::R32_SInt, VK_FORMAT_R32_SINT},
			{VertexFormat::R32_Float, VK_FORMAT_R32_SFLOAT},

			{VertexFormat::RG32_UInt, VK_FORMAT_R32G32_UINT},
			{VertexFormat::RG32_SInt, VK_FORMAT_R32G32_SINT},
			{VertexFormat::RG32_Float, VK_FORMAT_R32G32_SFLOAT},

			{VertexFormat::RGB32_UInt, VK_FORMAT_R32G32B32_UINT},
			{VertexFormat::RGB32_SInt, VK_FORMAT_R32G32B32_SINT},
			{VertexFormat::RGB32_Float, VK_FORMAT_R32G32B32_SFLOAT},

			{VertexFormat::RGBA32_UInt, VK_FORMAT_R32G32B32A32_UINT},
			{VertexFormat::RGBA32_SInt, VK_FORMAT_R32G32B32A32_SINT},
			{VertexFormat::RGBA32_Float, VK_FORMAT_R32G32B32A32_SFLOAT},

			// Packed / Special
			{VertexFormat::RGB10A2_UNorm, VK_FORMAT_A2B10G10R10_UNORM_PACK32},
			{VertexFormat::RGB10A2_UInt,  VK_FORMAT_A2B10G10R10_UINT_PACK32},
			{VertexFormat::R11G11B10_Float, VK_FORMAT_B10G11R11_UFLOAT_PACK32},
			{VertexFormat::R9G9B9E5_SharedExp, VK_FORMAT_E5B9G9R9_UFLOAT_PACK32},
		};

	public:
		static constexpr VkFormat toVkFormat(VertexFormat fmt) noexcept
		{
			for (auto m : mappings)
				if (m.vf == fmt) return m.vk;
			return VK_FORMAT_UNDEFINED;
		}

		int mapSemantic(Semantic s) const override
		{
			switch (s)
			{
			case Semantic::Position: return 0;
			case Semantic::Normal:   return 1;
			case Semantic::TexCoord: return 2;
			case Semantic::Color:    return 3;
			case Semantic::Tangent:  return 4;
			case Semantic::Bitangent:return 5;
			default: throw std::runtime_error("Vulkan: Unsupported semantic");
			}
		}
	};
#endif // defined(__ANDROID__) || defined(__linux__) || defined(_WIN64)

#if defined(__APPLE__)
	// =====================================================================
	// ---------------------- Metal маппер ---------------------------------
	// =====================================================================
	export struct MetalVertexFormatMapper final : IVertexFormatMapper
	{
	private:
		struct Mapping { VertexFormat vf; MTLPixelFormat mtl; };
		static constexpr Mapping mappings[] = {
			// 8-bit
			{VertexFormat::R8_UNorm, MTLPixelFormatR8Unorm},
			{VertexFormat::R8_SNorm, MTLPixelFormatInvalid},
			{VertexFormat::R8_UInt,  MTLPixelFormatR8Uint},
			{VertexFormat::R8_SInt,  MTLPixelFormatR8Sint},

			{VertexFormat::RG8_UNorm, MTLPixelFormatRG8Unorm},
			{VertexFormat::RG8_SNorm, MTLPixelFormatInvalid},
			{VertexFormat::RG8_UInt,  MTLPixelFormatRG8Uint},
			{VertexFormat::RG8_SInt,  MTLPixelFormatRG8Sint},

			{VertexFormat::RGBA8_UNorm, MTLPixelFormatRGBA8Unorm},
			{VertexFormat::RGBA8_SNorm, MTLPixelFormatInvalid},
			{VertexFormat::RGBA8_UInt,  MTLPixelFormatRGBA8Uint},
			{VertexFormat::RGBA8_SInt,  MTLPixelFormatRGBA8Sint},
			{VertexFormat::RGBA8_UNorm_sRGB, MTLPixelFormatRGBA8Unorm_sRGB},

			// 16-bit
			{VertexFormat::R16_UNorm, MTLPixelFormatR16Unorm},
			{VertexFormat::R16_SNorm, MTLPixelFormatInvalid},
			{VertexFormat::R16_UInt,  MTLPixelFormatR16Uint},
			{VertexFormat::R16_SInt,  MTLPixelFormatR16Sint},
			{VertexFormat::R16_Float, MTLPixelFormatR16Float},

			{VertexFormat::RG16_UNorm, MTLPixelFormatRG16Unorm},
			{VertexFormat::RG16_SNorm, MTLPixelFormatInvalid},
			{VertexFormat::RG16_UInt,  MTLPixelFormatRG16Uint},
			{VertexFormat::RG16_SInt,  MTLPixelFormatRG16Sint},
			{VertexFormat::RG16_Float, MTLPixelFormatRG16Float},

			{VertexFormat::RGBA16_UNorm, MTLPixelFormatRGBA16Unorm},
			{VertexFormat::RGBA16_SNorm, MTLPixelFormatInvalid},
			{VertexFormat::RGBA16_UInt,  MTLPixelFormatRGBA16Uint},
			{VertexFormat::RGBA16_SInt,  MTLPixelFormatRGBA16Sint},
			{VertexFormat::RGBA16_Float, MTLPixelFormatRGBA16Float},

			// 32-bit
			{VertexFormat::R32_UInt, MTLPixelFormatR32Uint},
			{VertexFormat::R32_SInt, MTLPixelFormatR32Sint},
			{VertexFormat::R32_Float, MTLPixelFormatR32Float},

			{VertexFormat::RG32_UInt, MTLPixelFormatRG32Uint},
			{VertexFormat::RG32_SInt, MTLPixelFormatRG32Sint},
			{VertexFormat::RG32_Float, MTLPixelFormatRG32Float},

			{VertexFormat::RGB32_UInt, MTLPixelFormatInvalid},
			{VertexFormat::RGB32_SInt, MTLPixelFormatInvalid},
			{VertexFormat::RGB32_Float, MTLPixelFormatInvalid},

			{VertexFormat::RGBA32_UInt, MTLPixelFormatRGBA32Uint},
			{VertexFormat::RGBA32_SInt, MTLPixelFormatRGBA32Sint},
			{VertexFormat::RGBA32_Float, MTLPixelFormatRGBA32Float},

			// Packed / Special
			{VertexFormat::RGB10A2_UNorm, MTLPixelFormatRGB10A2Unorm},
			{VertexFormat::RGB10A2_UInt,  MTLPixelFormatRGB10A2Uint},
			{VertexFormat::R11G11B10_Float, MTLPixelFormatRG11B10Float},
			{VertexFormat::R9G9B9E5_SharedExp, MTLPixelFormatRGB9E5Float},
		};

	public:
		static constexpr MTLPixelFormat toMTLFormat(VertexFormat fmt) noexcept
		{
			for (auto m : mappings)
				if (m.vf == fmt) return m.mtl;
			return MTLPixelFormatInvalid;
		}

		int mapSemantic(Semantic s) const override
		{
			switch (s)
			{
			case Semantic::Position: return 0;
			case Semantic::Normal:   return 1;
			case Semantic::TexCoord: return 2;
			case Semantic::Color:    return 3;
			case Semantic::Tangent:  return 4;
			case Semantic::Bitangent:return 5;
			default: throw std::runtime_error("Metal: Unsupported semantic");
			}
		}
	};
#endif // defined(__APPLE__)
}