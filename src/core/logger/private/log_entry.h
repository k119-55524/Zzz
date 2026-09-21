#pragma once

#include "../LoggerIncludes.h"
#include <core/serialize/Serializer.h>
#include "core/utils/LogCategory.h"

namespace zzz::logger
{
	using namespace zzz::core;
	struct LogEntry final : public ISerializable
	{
		uint64_t timestamp;
		eLogMessageType type;

		// Невладеющий указатель на глобальную (inline constexpr) категория логирования - без аллокации/копирования
		// имени категории на каждую запись лога. Категории имеют стабильный на весь процесс адрес (см. LogCategory.h).
		const LogCategory* category = &LogGeneral;

		std::string text;
		std::string file;
		std::string function;
		uint32_t line;

		LogEntry() = default;
		LogEntry(uint64_t ts, eLogMessageType tp, const LogCategory* cat, std::string txt, std::string f, std::string func, uint32_t l)
			: timestamp(ts), type(tp), category(cat), text(std::move(txt)), file(std::move(f)), function(std::move(func)), line(l) {}

	protected:
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const Serializer& serializer) const override;
		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& serializer) override;
	};
}
