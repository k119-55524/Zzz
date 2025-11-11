
#include "pch.h"

export module CPUVertexBuffer;

import Colors;

using namespace zzz::colors;

export namespace zzz::engineCore
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

		// Конструктор для Color
		Attribute(const Color& color) requires (S == Semantic::Color && std::is_same_v<T, float>)
		{
			for (size_t i = 0; i < N; ++i)
				value[i] = color[i];
		}
	};

	export struct LayoutEntry
	{
		Semantic semantic;
		VertexFormat format;
		size_t offset;
		uint32_t semanticIndex = 0;
	};

	// ----- Вершина -----
	export template<typename... Attrs>
		struct MeshData : public Attrs...
	{
		using Attributes = std::tuple<Attrs...>;

		template<typename A>
		A& get() { return static_cast<A&>(*this); }

		template<typename A>
		const A& get() const { return static_cast<const A&>(*this); }

		template<typename A>
		static constexpr size_t offsetOf()
		{
			return reinterpret_cast<size_t>(&(reinterpret_cast<const MeshData*>(0)->A::value));
		}

		static auto layout()
		{
			constexpr size_t count = sizeof...(Attrs);
			std::array<LayoutEntry, count> arr{};

			// будем считать, сколько раз каждая семантика уже встречалась
			std::array<uint32_t, static_cast<size_t>(Semantic::Bitangent) + 1> counters{};
			counters.fill(0);

			size_t i = 0;
			([&] {
				using A = Attrs;
				arr[i] = {
					A::semantic,
					deduceFormat<A>(),
					offsetOf<A>(),
					counters[static_cast<size_t>(A::semantic)]++
				};
				++i;
				}(), ...);

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

		virtual size_t Stride() const = 0;
		virtual size_t SizeInBytes() const = 0;
		virtual const void* GetData() const = 0;
		virtual size_t VertexCount() const = 0;

		// Удаляем локальное определение LayoutEntry
		virtual std::vector<LayoutEntry> layout() const = 0;
	};

	// ----- Шаблон буфера вершин -----
	export template<typename... Attrs>
		struct CPUVertexBuffer : public ICPUVertexBuffer
	{
		using VertexT = MeshData<Attrs...>;
		std::vector<VertexT> vertices;

		CPUVertexBuffer() = default;
		CPUVertexBuffer(std::initializer_list<VertexT> init) : vertices(init) {}
		~CPUVertexBuffer() = default;

		CPUVertexBuffer(CPUVertexBuffer&&) noexcept = default;
		CPUVertexBuffer& operator=(CPUVertexBuffer&&) noexcept = default;

		CPUVertexBuffer(const CPUVertexBuffer&) = delete;
		CPUVertexBuffer& operator=(const CPUVertexBuffer&) = delete;

		size_t Stride() const override { return sizeof(VertexT); }
		size_t SizeInBytes() const override { return vertices.size() * sizeof(VertexT); }
		size_t VertexCount() const override { return vertices.size(); }
		const void* GetData() const override { return vertices.data(); }

		std::vector<LayoutEntry> layout() const override
		{
			auto arr = VertexT::layout();
			return { arr.begin(), arr.end() };
		}
	};

	// ----- Готовые типы -----
	export using VB_P3N3 = CPUVertexBuffer<
		Attribute<Semantic::Position, float, 3>,
		Attribute<Semantic::Normal, float, 3>>;

	export using VB_P3C3 = CPUVertexBuffer<
		Attribute<Semantic::Position, float, 3>,
		Attribute<Semantic::Color, float, 3>>;

	export using VB_P3N3T3 = CPUVertexBuffer<
		Attribute<Semantic::Position, float, 3>,
		Attribute<Semantic::Normal, float, 3>,
		Attribute<Semantic::TexCoord, float, 2>>;
}