#pragma once

#include <string_view>
#include <stdexcept>
#include "eLogMessageType.h"
#include "eWinResize.h"

namespace zzz::common
{
	class EnumToString
	{
	public:
		static constexpr std::string_view ToString(eLogMessageType type)
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
			throw std::invalid_argument("Unhandled eLogMessageType");
		}

		static constexpr std::string_view ToString(eWinResize type)
		{
			switch (type)
			{
			case eWinResize::Show:   return "SHOW";
			case eWinResize::Hide:   return "HIDE";
			case eWinResize::Resize: return "RESIZE";
			}
			throw std::invalid_argument("Unhandled eWinResize");
		}
	};
}
