#pragma once

#include <core/serialize/Serializer.h>

#include "../LoggerIncludes.h"

namespace zzz::logger
{
	using namespace zzz::core;
	struct LogEntry final : public ISerializable
	{
		uint64_t timestamp;
		eLogMessageType type;
		std::string text;
		std::string file;
		std::string function;
		uint32_t line;

		LogEntry() = default;
		LogEntry(uint64_t ts, eLogMessageType tp, std::string txt, std::string f, std::string func, uint32_t l)
			: timestamp(ts), type(tp), text(std::move(txt)), file(std::move(f)), function(std::move(func)), line(l) {}

	protected:
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const Serializer& serializer) const override;
		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& serializer) override;
	};
}
