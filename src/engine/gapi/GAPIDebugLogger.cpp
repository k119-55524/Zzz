
#if Z_GAPI_VERBOSE_DEBUG_LAYER
#include "core/utils/Macroses.h"
#include "core/enums/eEnumToString.h"
#include "engine/gapi/GAPIDebugLogger.h"

namespace zzz::engine
{
	void GAPIDebugLogger::Report(eGAPIType backend, eLogMessageType severity, std::string_view message)
	{
		if (message.empty())
			return;

		switch (severity)
		{
		case eLogMessageType::Warning:
			DOutWarning(GAPIVerbose, "[GAPI:{}] {}", EnumToString::ToString(backend), message);
			break;
		case eLogMessageType::Error:
			DOutError(GAPIVerbose, "[GAPI:{}] {}", EnumToString::ToString(backend), message);
			break;
		case eLogMessageType::Exception:
			DOutException(GAPIVerbose, "[GAPI:{}] {}", EnumToString::ToString(backend), message);
			break;
		case eLogMessageType::Critical:
			DOutCritical(GAPIVerbose, "[GAPI:{}] {}", EnumToString::ToString(backend), message);
			break;
		case eLogMessageType::Fatal:
			DOutFatal(GAPIVerbose, "[GAPI:{}] {}", EnumToString::ToString(backend), message);
			break;
		case eLogMessageType::Message:
		default:
			DOut(GAPIVerbose, "[GAPI:{}] {}", EnumToString::ToString(backend), message);
			break;
		}
	}
}
#endif // Z_GAPI_VERBOSE_DEBUG_LAYER
