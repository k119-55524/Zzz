#pragma once

#include <string_view>
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
			case eLogMessageType::Message:   return "Message";
			case eLogMessageType::Warning:   return "Warning";
			case eLogMessageType::Error:     return "Error";
			case eLogMessageType::Exception: return "Exception";
			case eLogMessageType::Critical:  return "Critical";
			case eLogMessageType::Fatal:     return "Fatal";
			case eLogMessageType::All:       return "All";
			case eLogMessageType::None:      return "None";
			default:                         return "Unknown";
			}
		}

		static constexpr std::string_view ToString(eWinResize type)
		{
			switch (type)
			{
			case eWinResize::Show:   return "Show";
			case eWinResize::Hide:   return "Hide";
			case eWinResize::Resize: return "Resize";
			default:                 return "Unknown";
			}
		}
	};
}
