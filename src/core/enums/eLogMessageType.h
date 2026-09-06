#pragma once

#include <string_view>
#include "math/utils/Types.h"
#include "core/utils/ThrowWrappers.h"

namespace zzz::core
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

	constexpr std::string_view ToString(eLogMessageType type)
	{
		switch (type)
		{
		case eLogMessageType::Message:   return "MESSAGE";
		case eLogMessageType::Warning:   return "WARNING";
		case eLogMessageType::Error:     return "ERROR";
		case eLogMessageType::Exception: return "EXCEPTION";
		case eLogMessageType::Critical:  return "CRITICAL";
		case eLogMessageType::Fatal:     return "FATAL";
		case eLogMessageType::All:       return "ALL";
		case eLogMessageType::None:      return "NONE";
		}
		THROW_RUNTIME("Необработанный eLogMessageType");
	}

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
