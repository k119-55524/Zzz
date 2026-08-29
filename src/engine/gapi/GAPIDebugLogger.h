#pragma once

#include "core/enums/eGAPIType.h"
#include "core/enums/eLogMessageType.h"

namespace zzz::engine
{
	using namespace zzz::core;

	// Единая точка подробного вывода логов GAPI(отладочный слой)
	class GAPIDebugLogger
	{
	public:
#if Z_GAPI_VERBOSE_DEBUG_LAYER
		static void Report(eGAPIType backend, eLogMessageType severity, std::string_view message);
#else
		static void Report(eGAPIType, eLogMessageType, std::string_view) {}
#endif
	};
}
