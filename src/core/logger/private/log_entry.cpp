
#include "private/log_entry.h"

namespace zzz::logger
{
	using namespace zzz::core;

	// Формат протокола v2 (см. c_LogProtocolVersion, Constants.h):
	// [Version, Timestamp, Type, CategoryGroup, CategoryName, CategoryIsGuaranteed, Text, File, Function, Line]
	[[nodiscard]] std::expected<void, std::string> LogEntry::Serialize(std::vector<std::byte>& buffer, const Serializer& serializer) const
	{
		const LogCategory& cat = category ? *category : LogGeneral;

		if (auto res = serializer.Serialize(buffer, c_LogProtocolVersion); !res) return res;
		if (auto res = serializer.Serialize(buffer, timestamp); !res) return res;
		if (auto res = serializer.Serialize(buffer, static_cast<uint64_t>(type)); !res) return res;
		if (auto res = serializer.Serialize(buffer, cat.group); !res) return res;
		if (auto res = serializer.Serialize(buffer, std::string(cat.name)); !res) return res;
		if (auto res = serializer.Serialize(buffer, cat.isGuaranteed); !res) return res;
		if (auto res = serializer.Serialize(buffer, text); !res) return res;
		if (auto res = serializer.Serialize(buffer, file); !res) return res;
		if (auto res = serializer.Serialize(buffer, function); !res) return res;
		if (auto res = serializer.Serialize(buffer, line); !res) return res;
		return {};
	}

	[[nodiscard]] std::expected<void, std::string> LogEntry::Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& serializer)
	{
		uint32_t protocolVersion = 0;
		if (auto res = serializer.Deserialize(buffer, offset, protocolVersion); !res) return res;

		if (auto res = serializer.Deserialize(buffer, offset, timestamp); !res) return res;
		uint64_t type_raw = 0;
		if (auto res = serializer.Deserialize(buffer, offset, type_raw); !res) return res;
		type = static_cast<eLogMessageType>(type_raw);

		// В движке нет рантайм-реестра категорий по имени (категории - inline constexpr объекты, их адрес известен
		// только на этапе компиляции), поэтому восстановить исходный const LogCategory* здесь невозможно. Deserialize
		// нужен лишь для соответствия интерфейсу ISerializable - в проде не используется: сетевой поток независимо
		// разбирает RemoteLogViewer (см. NetworkReceiver.cs), которому указатель не нужен - только сами данные категории.
		eLogCategoryGroup group{};
		if (auto res = serializer.Deserialize(buffer, offset, group); !res) return res;
		std::string categoryName;
		if (auto res = serializer.Deserialize(buffer, offset, categoryName); !res) return res;
		bool isGuaranteed = false;
		if (auto res = serializer.Deserialize(buffer, offset, isGuaranteed); !res) return res;
		(void)protocolVersion; (void)group; (void)categoryName; (void)isGuaranteed;
		category = &LogGeneral;

		if (auto res = serializer.Deserialize(buffer, offset, text); !res) return res;
		if (auto res = serializer.Deserialize(buffer, offset, file); !res) return res;
		if (auto res = serializer.Deserialize(buffer, offset, function); !res) return res;
		if (auto res = serializer.Deserialize(buffer, offset, line); !res) return res;
		return {};
	}
}
