
#include "private/log_entry.h"

#if Z_ADD_LOGGER || Z_DEVELOPMENT_BUILD
namespace zzz::logger
{
	using namespace zzz::core;
	[[nodiscard]] std::expected<void, std::string> LogEntry::Serialize(std::vector<std::byte>& buffer, const Serializer& serializer) const
	{
		if (auto res = serializer.Serialize(buffer, timestamp); !res) return res;
		if (auto res = serializer.Serialize(buffer, static_cast<uint64_t>(type)); !res) return res;
		if (auto res = serializer.Serialize(buffer, text); !res) return res;
		if (auto res = serializer.Serialize(buffer, file); !res) return res;
		if (auto res = serializer.Serialize(buffer, function); !res) return res;
		if (auto res = serializer.Serialize(buffer, line); !res) return res;
		return {};
	}

	[[nodiscard]] std::expected<void, std::string> LogEntry::Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& serializer)
	{
		if (auto res = serializer.Deserialize(buffer, offset, timestamp); !res) return res;
		uint64_t type_raw = 0;
		if (auto res = serializer.Deserialize(buffer, offset, type_raw); !res) return res;
		type = static_cast<eLogMessageType>(type_raw);
		if (auto res = serializer.Deserialize(buffer, offset, text); !res) return res;
		if (auto res = serializer.Deserialize(buffer, offset, file); !res) return res;
		if (auto res = serializer.Deserialize(buffer, offset, function); !res) return res;
		if (auto res = serializer.Deserialize(buffer, offset, line); !res) return res;
		return {};
	}
}
#endif