#pragma once

#include "log_entry.h"

namespace zzz::logger
{
	class IBroadcaster
	{
	public:
		virtual ~IBroadcaster() = default;
		virtual void OnLog(const LogEntry& entry) = 0;
		virtual void PushLogsBatch(std::span<const LogEntry> entries)
		{
			for (const auto& entry : entries)
				OnLog(entry);
		}
		virtual void SetMaxQueueSize(zU32 /*newSize*/) {}
	};
}
