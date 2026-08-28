#include "engine/gapi/GAPIDebugLogger.h"
#include "core/utils/macros/GAPILogMacros.h"

#if Z_DEBUG_BUILD || Z_DEVELOPMENT_BUILD
namespace zzz::engine
{
	namespace
	{
		bool IsReportFlagEnabled(std::string_view flag)
		{
			for (const char* enabled : c_GAPIDebugReportFlags)
			{
				if (flag == enabled)
					return true;
			}
			return false;
		}

		// Переводит severity/category в тег из c_GAPIDebugReportFlags ("error"/"warn"/"perf"/"info"/"debug") -
		// единая точка фильтрации, одинаковая для Vulkan и DirectX12 (см. комментарий у c_GAPIDebugReportFlags
		// в Constants.h и у класса GAPIDebugLogger в GAPIDebugLogger.h).
		bool ShouldReport(eGAPIDebugSeverity severity, eGAPIDebugCategory category)
		{
			if (category == eGAPIDebugCategory::Performance && IsReportFlagEnabled("perf"))
				return true;

			switch (severity)
			{
			case eGAPIDebugSeverity::Error:   return IsReportFlagEnabled("error");
			case eGAPIDebugSeverity::Warning: return IsReportFlagEnabled("warn");
			case eGAPIDebugSeverity::Info:    return IsReportFlagEnabled("info");
			case eGAPIDebugSeverity::Verbose: return IsReportFlagEnabled("debug");
			default: return false;
			}
		}
	}

	void GAPIDebugLogger::Report(eGAPIType backend, eGAPIDebugSeverity severity, eGAPIDebugCategory category, std::string_view message)
	{
		if (message.empty())
			return;

		if (!ShouldReport(severity, category))
			return;

		switch (severity)
		{
		case eGAPIDebugSeverity::Error:
			DOutErrorGAPI(">#### [GAPI:{}] [{}] {}", EnumToString::ToString(backend), EnumToString::ToString(category), message);
			break;
		case eGAPIDebugSeverity::Warning:
			DOutWarningGAPI(">#### [GAPI:{}] [{}] {}", EnumToString::ToString(backend), EnumToString::ToString(category), message);
			break;
		case eGAPIDebugSeverity::Info:
		case eGAPIDebugSeverity::Verbose:
		default:
			DOutGAPI(">#### [GAPI:{}] [{} | {}] {}", EnumToString::ToString(backend), EnumToString::ToString(category), EnumToString::ToString(severity), message);
			break;
		}
	}
}
#endif // Z_DEBUG_BUILD || Z_DEVELOPMENT_BUILD
