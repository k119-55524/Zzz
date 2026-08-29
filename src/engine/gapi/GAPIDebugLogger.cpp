
#include "core/utils/Macroses.h"
#include "core/enums/eEnumToString.h"
#include "engine/gapi/GAPIDebugLogger.h"

#if Z_ADD_LOGGER
namespace zzz::engine
{
	void GAPIDebugLogger::Report(eGAPIType backend, eLogMessageType severity, std::string_view message)
	{
		if (message.empty())
			return;

		// Guaranteed-уровни (всё кроме Message) идут в LogGAPI - гарантированную категорию, которая всегда
		// доходит до IDE независимо от рантайм-фильтра (см. Z_LOG_DISPATCH, ApplyFilter=false). Message
		// (Info/Verbose нативного API) идёт в LogGAPIVerbose - обычную фильтруемую категорию, т.к. это
		// потенциально очень шумный поток, который не должен быть гарантированным.
		//
		// Формат сообщения намеренно минимален - Logger::MakeLogMessage/MakeLogMessageError уже добавляют
		// "[{type}] [{category}]" сами (см. logger.cpp), здесь остаётся только различить бэкенд (Vulkan/D3D12/...).
		switch (severity)
		{
		case eLogMessageType::Warning:
			DOutWarning(LogGAPI, "[GAPI:{}] {}", EnumToString::ToString(backend), message);
			break;
		case eLogMessageType::Error:
			DOutError(LogGAPI, "[GAPI:{}] {}", EnumToString::ToString(backend), message);
			break;
		case eLogMessageType::Exception:
			DOutException(LogGAPI, "[GAPI:{}] {}", EnumToString::ToString(backend), message);
			break;
		case eLogMessageType::Critical:
			DOutCritical(LogGAPI, "[GAPI:{}] {}", EnumToString::ToString(backend), message);
			break;
		case eLogMessageType::Fatal:
			DOutFatal(LogGAPI, "[GAPI:{}] {}", EnumToString::ToString(backend), message);
			break;
		case eLogMessageType::Message:
		default:
			DOut(LogGAPIVerbose, "[GAPI:{}] {}", EnumToString::ToString(backend), message);
			break;
		}
	}
}
#endif // Z_ADD_LOGGER
