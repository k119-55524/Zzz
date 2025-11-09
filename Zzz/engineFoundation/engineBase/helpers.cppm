
#include "pch.h"

export module helpers;

namespace zzz::helpers
{
	// Универсальная функция округления вверх до кратного ALIGN
	template <uint64_t ALIGN, typename T>
	constexpr T CalcAlignedSize(T size)
	{
		static_assert(ALIGN > 0 && (ALIGN & (ALIGN - 1)) == 0,
			"ALIGN must be a power of 2");

		static_assert(std::is_unsigned_v<T>, "T must be unsigned integer type");

		if (size > std::numeric_limits<T>::max() - (ALIGN - 1)) {
			return std::numeric_limits<T>::max(); // защита от переполнения
		}

		return (size + (ALIGN - 1)) & ~(ALIGN - 1);
	}

	// Для 32-бит
	using Size32 = uint32_t;
	export constexpr Size32 CalcBufferSize32(Size32 size) { return CalcAlignedSize<256>(size); }

	// Для 64-бит (D3D12, большие буферы)
	using Size64 = uint64_t;
	export constexpr Size64 CalcBufferSize64(Size64 size) { return CalcAlignedSize<256>(size); }
}