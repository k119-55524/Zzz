#pragma once

#include "log_entry.h"

#if Z_ADD_LOGGER || Z_DEVELOPMENT_BUILD

namespace zzz::logger
{
	class IBroadcaster
	{
	public:
		virtual ~IBroadcaster() = default;
		virtual void OnLog(const LogEntry& entry) = 0;
	};
}

#endif
