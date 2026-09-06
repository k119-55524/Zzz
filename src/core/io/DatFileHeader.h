#pragma once

#include <array>
#include <span>
#include <string>
#include <vector>
#include <cstddef>
#include <expected>
#include <logger/logger.h>

#include "core/utils/Version.h"
#include "core/serialize/Serializer.h"
#include "core/constants/LogCategoryConstants.h"

namespace zzz::core
{
	/**
	 * @brief Универсальный заголовок любого бинарного .dat файла движка (27 байт).
	 * Включает:
	 * - m_Magic сигнатура ("ZZP", "ZZD", "ZZZ")
	 * - m_Version
	 * - m_EntryCount к-во записей в пакете (для пакетов > 0, для user.dat = 0)
	 * - m_Timestamp время сборки пакета или время сохранения конфигурации
	 */
	class DatFileHeader final : public ISerializable
	{
	public:
		using Magic = std::array<std::byte, 3>;

		constexpr DatFileHeader() = default;
		constexpr DatFileHeader(const Magic& magic, const Version& version, zU32 entryCount, zU64 timestamp) noexcept
			: m_Magic(magic)
			, m_Version(version)
			, m_EntryCount(entryCount)
			, m_Timestamp(timestamp)
		{}
		[[nodiscard]] constexpr const Magic& GetMagic() const noexcept { return m_Magic; }
		[[nodiscard]] constexpr const Version& GetVersion() const noexcept { return m_Version; }
		[[nodiscard]] constexpr zU32 GetEntryCount() const noexcept { return m_EntryCount; }
		[[nodiscard]] constexpr zU64 GetTimestamp() const noexcept { return m_Timestamp; }
		[[nodiscard]] constexpr zU64 GetBuildTime() const noexcept { return m_Timestamp; }

		/// @brief Размер бинарного представления DatFileHeader (3 + 12 + 4 + 8 = 27 байт).
		[[nodiscard]] static constexpr std::size_t BinarySize() noexcept
		{
			return sizeof(Magic) + Version::BinarySize() + sizeof(zU32) + sizeof(zU64);
		}

		[[nodiscard]] static std::string MagicToString(const Magic& magic)
		{
			std::string str;
			str.reserve(magic.size());
			for (const auto& b : magic)
				str.push_back(static_cast<char>(b));
			return str;
		}

		[[nodiscard]] std::string GetMagicString() const
		{
			return MagicToString(m_Magic);
		}

		[[nodiscard]] std::expected<void, std::string> Validate(
			const Magic& expectedMagic,
			zU8 expectedMajor) const
		{
			if (m_Magic != expectedMagic)
				return UNEXPECTED("Некорректная сигнатура заголовка");

			if (m_Version.GetMajor() != expectedMajor)
				return UNEXPECTED("Несовместимая версия формата: {} (ожидалась мажорная версия {}).", m_Version.ToString(), expectedMajor);

			return {};
		}

		[[nodiscard]] static std::string FormatTimestamp(zU64 timestamp)
		{
			if (timestamp == 0)
				return "0";

			const auto timePt = std::chrono::time_point<std::chrono::system_clock>(std::chrono::milliseconds(timestamp));
			const auto in_time_t = std::chrono::system_clock::to_time_t(timePt);
			std::tm bt{};
#if defined(_WIN32)
			localtime_s(&bt, &in_time_t);
#else
			localtime_r(&in_time_t, &bt);
#endif
			const auto ms = timestamp % 1000;
			return std::format("{:04}-{:02}-{:02} {:02}:{:02}:{:02}:{:03}",
				bt.tm_year + 1900, bt.tm_mon + 1, bt.tm_mday,
				bt.tm_hour, bt.tm_min, bt.tm_sec, ms);
		}

		[[nodiscard]] std::string GetFormattedTimestamp() const
		{
			return FormatTimestamp(m_Timestamp);
		}

		inline void LogFileBlock(std::string_view prefix = "") const
		{
#if Z_ADD_LOGGER
			DOut(::zzz::core::Assets, "{}[DatFileHeader]", prefix);
			DOut(::zzz::core::Assets, "{}  magic: {}", prefix, GetMagicString());
			DOut(::zzz::core::Assets, "{}  version: {}", prefix, m_Version.ToString());
			DOut(::zzz::core::Assets, "{}  entryCount: {}", prefix, m_EntryCount);
			DOut(::zzz::core::Assets, "{}  timestamp: {}", prefix, GetFormattedTimestamp());
#endif // Z_ADD_LOGGER
		}

		[[nodiscard]] constexpr bool operator==(const DatFileHeader& other) const noexcept
		{
			return m_Magic == other.m_Magic &&
			       m_Version == other.m_Version &&
			       m_EntryCount == other.m_EntryCount &&
			       m_Timestamp == other.m_Timestamp;
		}

	protected:
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const Serializer& serializer) const override
		{
			return serializer.Serialize(buffer, m_Magic)
				.and_then([&]() { return serializer.Serialize(buffer, m_Version); })
				.and_then([&]() { return serializer.Serialize(buffer, m_EntryCount); })
				.and_then([&]() { return serializer.Serialize(buffer, m_Timestamp); });
		}

		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& serializer) override
		{
			return serializer.Deserialize(buffer, offset, m_Magic)
				.and_then([&]() { return serializer.Deserialize(buffer, offset, m_Version); })
				.and_then([&]() { return serializer.Deserialize(buffer, offset, m_EntryCount); })
				.and_then([&]() { return serializer.Deserialize(buffer, offset, m_Timestamp); });
		}

	private:
		Magic m_Magic{};
		Version m_Version{};
		zU32 m_EntryCount = 0;
		zU64 m_Timestamp = 0;
	};
}
