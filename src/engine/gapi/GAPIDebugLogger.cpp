
#if Z_GAPI_VERBOSE_DEBUG_LAYER
#include "core/utils/Macroses.h"
#include "engine/gapi/GAPIDebugLogger.h"

Z_SET_LOG_CATEGORY(::zzz::core::GAPI);

namespace zzz::engine
{
	void GAPIDebugLogger::Report(eGAPIType backend, eLogMessageType severity, std::string_view message)
	{
		if (message.empty())
			return;

		switch (severity)
		{
		case eLogMessageType::Warning:
			DOutWarning(GAPIVerbose, "[GAPI:{}] {}", ToString(backend), message);
			break;
		case eLogMessageType::Error:
			DOutError(GAPIVerbose, "[GAPI:{}] {}", ToString(backend), message);
			break;
		case eLogMessageType::Exception:
			DOutException(GAPIVerbose, "[GAPI:{}] {}", ToString(backend), message);
			break;
		case eLogMessageType::Critical:
			DOutCritical(GAPIVerbose, "[GAPI:{}] {}", ToString(backend), message);
			break;
		case eLogMessageType::Fatal:
			DOutFatal(GAPIVerbose, "[GAPI:{}] {}", ToString(backend), message);
			break;
		case eLogMessageType::Message:
		default:
			DOut(GAPIVerbose, "[GAPI:{}] {}", ToString(backend), message);
			break;
		}
	}
}
#endif // Z_GAPI_VERBOSE_DEBUG_LAYER
