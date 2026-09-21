#pragma once

#include "IBroadcaster.h"

namespace zzz::logger
{
	class CallbackBroadcaster : public IBroadcaster
	{
	public:
		CallbackBroadcaster(LogCallback callback) : m_Callback(callback) {}

		void OnLog(const LogEntry& entry) override
		{
			if (m_Callback)
			{
				LogCallbackEntry cbEntry;
				cbEntry.timestamp = entry.timestamp;
				cbEntry.type = static_cast<int>(entry.type);
				cbEntry.text = entry.text.c_str();
				cbEntry.file = entry.file.c_str();
				cbEntry.function = entry.function.c_str();
				cbEntry.line = entry.line;
				m_Callback(cbEntry);
			}
		}

	private:
		LogCallback m_Callback;
	};
}
