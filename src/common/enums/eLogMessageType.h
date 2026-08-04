#pragma once

#include <core/utils/Types.h>

namespace zzz::common
{
	enum class eLogMessageType : zU8
	{
		None      = 0,
		Message   = 1 << 0,
		Warning   = 1 << 1,
		Error     = 1 << 2,
		Exception = 1 << 3,
		Critical  = 1 << 4,
		Fatal     = 1 << 5,
		All       = 0xFF
	};

	inline constexpr eLogMessageType operator|(eLogMessageType a, eLogMessageType b)
	{
		return static_cast<eLogMessageType>(static_cast<zU8>(a) | static_cast<zU8>(b));
	}

	inline constexpr eLogMessageType operator&(eLogMessageType a, eLogMessageType b)
	{
		return static_cast<eLogMessageType>(static_cast<zU8>(a) & static_cast<zU8>(b));
	}

	inline constexpr bool operator!(eLogMessageType a)
	{
		return static_cast<zU8>(a) == 0;
	}
}
