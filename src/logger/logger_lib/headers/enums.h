#pragma once

#include <common/types.h>

namespace zzz::logger
{
	enum class eLogMessageType : zzz::common::zU8
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
		return static_cast<eLogMessageType>(static_cast<zzz::common::zU8>(a) | static_cast<zzz::common::zU8>(b));
	}

	inline constexpr eLogMessageType operator&(eLogMessageType a, eLogMessageType b)
	{
		return static_cast<eLogMessageType>(static_cast<zzz::common::zU8>(a) & static_cast<zzz::common::zU8>(b));
	}

	inline constexpr bool operator!(eLogMessageType a)
	{
		return static_cast<zzz::common::zU8>(a) == 0;
	}
}
