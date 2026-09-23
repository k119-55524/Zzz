#pragma once

#include <span>
#include <array>
#include <chrono>
#include <format>
#include <limits>
#include <string>
#include <vector>
#include <cstddef>
#include <expected>
#include <string_view>

#include "core/utils/Version.h"
#include "core/logger/logger.h"
#include "core/serialize/Serializer.h"
#include "core/utils/macros/MiscMacros.h"
#include "core/constants/LogCategoryConstants.h"
#include "core/constants/PackagesConstants.h"

namespace zzz::core
{
	/**
	 * @brief Универсальный заголовок любого бинарного .dat файла движка (31 байт).
	 *
	 * Структура заголовка:
	 * - m_Magic (3 байта) — сигнатура формата (например, "ZPD", "ZDD", "ZUD")
	 * - m_HeaderSize (4 байта) — полный размер заголовка на диске (c_FullHeaderSize = 31 байт)
	 * - m_Version (12 байт: major 4, minor 4, patch 4) — версия формата
	 * - m_EntryCount (4 байта) — к-во записей в пакете (для пакетов > 0, для конфигурации = 0)
	 * - m_Timestamp (8 байт) — время сборки пакета или время сохранения конфигурации
	 */
	class DatFileHeader final : public ISerializable
	{
	public:
		using Magic = DatMagic;

		/// @brief Размер конверта заголовка без сигнатуры (4 + 12 + 4 + 8 = 28 байт).
		static constexpr zU32 c_EnvelopeSize = static_cast<zU32>(sizeof(zU32) + Version::BinarySize() + sizeof(zU32) + sizeof(zU64));

		/// @brief Базовый размер заголовка по умолчанию для стандартных 3-байтовых пакетов (31 байт).
		static constexpr zU32 c_BaseHeaderSize = 3 + c_EnvelopeSize;

		/// @brief Размер дополнительных данных формата (в текущей архитектуре равен 0, задел на будущее).
		static constexpr zU32 c_ExtraDataSize = 0;

		/// @brief Полный размер заголовка с учётом дополнительных данных (compile-time, 31 байт).
		static constexpr zU32 c_FullHeaderSize = c_BaseHeaderSize + c_ExtraDataSize;

		/// @brief Конструктор по умолчанию.
		constexpr DatFileHeader() noexcept = default;

		/// @brief Конструктор из единого описателя формата DatFileFormat (сигнатура + версия).
		constexpr DatFileHeader(
			const DatFileFormat& format,
			zU32 entryCount = 0,
			zU64 timestamp = 0) noexcept :
			m_Magic{ format.Magic },
			m_HeaderSize{ static_cast<zU32>(format.Magic.size() + c_EnvelopeSize) },
			m_Version{ format.FormatVersion },
			m_EntryCount{ entryCount },
			m_Timestamp{ timestamp }
		{}

		/// @brief Конструктор с инициализацией сигнатуры, версии и опциональных полей.
		constexpr DatFileHeader(
			const Magic& magic,
			const Version& version,
			zU32 entryCount = 0,
			zU64 timestamp = 0) noexcept :
			m_Magic{ magic },
			m_HeaderSize{ static_cast<zU32>(magic.size() + c_EnvelopeSize) },
			m_Version{ version },
			m_EntryCount{ entryCount },
			m_Timestamp{ timestamp }
		{}

		[[nodiscard]] constexpr const Magic& GetMagic() const noexcept { return m_Magic; }
		[[nodiscard]] constexpr zU32 GetHeaderSize() const noexcept { return m_HeaderSize; }
		[[nodiscard]] constexpr const Version& GetVersion() const noexcept { return m_Version; }
		[[nodiscard]] constexpr zU32 GetEntryCount() const noexcept { return m_EntryCount; }
		[[nodiscard]] constexpr zU64 GetTimestamp() const noexcept { return m_Timestamp; }

		/**
		 * @brief Десериализует заголовок из среза байт файла и валидирует его.
		 * Сверяет прочитанные Magic и Version с ожидаемыми значениями, уже сохранёнными в этом экземпляре (*this).
		 * @param headBytes Срез байт начала файла (минимум базовый размер).
		 * @param fileSize Полный физический размер файла (для проверки headerSize <= fileSize).
		 * @return Успех или сообщение об ошибке. Смещение TOC доступно через GetHeaderSize().
		 */
		[[nodiscard]] std::expected<void, std::string> DeserializeAndValidate(
			std::span<const std::byte> headBytes,
			std::uintmax_t fileSize = (std::numeric_limits<std::uintmax_t>::max)())
		{
			const std::size_t minExpectedSize = m_Magic.size() + c_EnvelopeSize;
			if (headBytes.size() < minExpectedSize)
			{
				return UNEXPECTED("Буфер заголовка меньше базового размера: {} < {} байт",
					headBytes.size(), minExpectedSize);
			}

			Serializer serializer;
			std::size_t offset = 0;
			if (auto res = serializer.Deserialize(headBytes, offset, *this); !res)
			{
				return UNEXPECTED("Ошибка десериализации заголовка: {}", res.error());
			}

			if (static_cast<std::uintmax_t>(m_HeaderSize) > fileSize)
			{
				return UNEXPECTED("Размер заголовка ({} байт) выходит за пределы размера файла ({} байт)",
					m_HeaderSize, fileSize);
			}

			return {};
		}

		inline void LogFileBlock([[maybe_unused]] std::string_view prefix = "") const
		{
#if Z_ADD_LOGGER
			std::string formattedTime = "0";
			if (m_Timestamp != 0)
			{
				const auto timePt = std::chrono::time_point<std::chrono::system_clock>(std::chrono::milliseconds(m_Timestamp));
				const auto in_time_t = std::chrono::system_clock::to_time_t(timePt);
				std::tm bt{};
#if defined(_WIN32)
				localtime_s(&bt, &in_time_t);
#else
				localtime_r(&in_time_t, &bt);
#endif
				const auto ms = m_Timestamp % 1000;
				formattedTime = std::format("{:04}-{:02}-{:02} {:02}:{:02}:{:02}:{:03}",
					bt.tm_year + 1900, bt.tm_mon + 1, bt.tm_mday,
					bt.tm_hour, bt.tm_min, bt.tm_sec, ms);
			}

			DOut(Assets, "{}[DatFileHeader]", prefix);
			DOut(Assets, "{}  magic: {}", prefix, m_Magic);
			DOut(Assets, "{}  headerSize: {}", prefix, m_HeaderSize);
			DOut(Assets, "{}  version: {}", prefix, m_Version.ToString());
			DOut(Assets, "{}  entryCount: {}", prefix, m_EntryCount);
			DOut(Assets, "{}  timestamp: {}", prefix, formattedTime);
#endif // Z_ADD_LOGGER
		}

		[[nodiscard]] constexpr auto operator<=>(const DatFileHeader& other) const noexcept
		{
			if (auto cmp = m_Magic <=> other.m_Magic; cmp != 0) return cmp;
			if (auto cmp = m_HeaderSize <=> other.m_HeaderSize; cmp != 0) return cmp;
			if (auto cmp = m_Version <=> other.m_Version; cmp != 0) return cmp;
			if (auto cmp = m_EntryCount <=> other.m_EntryCount; cmp != 0) return cmp;
			return m_Timestamp <=> other.m_Timestamp;
		}

		[[nodiscard]] constexpr bool operator==(const DatFileHeader& other) const noexcept
		{
			return (*this <=> other) == 0;
		}

	protected:
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const Serializer& serializer) const override
		{
			if (!m_Magic.empty())
			{
				auto res = serializer.Serialize(buffer, std::as_bytes(std::span(m_Magic)));
				if (!res)
					return res;
			}

			return serializer.Serialize(buffer, m_HeaderSize)
				.and_then([&]() { return serializer.Serialize(buffer, m_Version); })
				.and_then([&]() { return serializer.Serialize(buffer, m_EntryCount); })
				.and_then([&]() { return serializer.Serialize(buffer, m_Timestamp); });
		}

		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& serializer) override
		{
			const std::size_t startOffset = offset;

			if (!m_Magic.empty())
			{
				if (offset + m_Magic.size() > buffer.size())
				{
					return UNEXPECTED("Буфер слишком мал для чтения сигнатуры (требуется {} байт)", m_Magic.size());
				}

				const std::string_view readMagicStr(reinterpret_cast<const char*>(buffer.data() + offset), m_Magic.size());
				offset += m_Magic.size();

				if (m_Magic != readMagicStr)
				{
					return UNEXPECTED("Некорректная сигнатура заголовка: '{}' (ожидалась '{}')",
						readMagicStr, m_Magic);
				}
			}

			zU32 readHeaderSize = 0;
			Version readVersion{};
			zU32 readEntryCount = 0;
			zU64 readTimestamp = 0;

			auto res = serializer.Deserialize(buffer, offset, readHeaderSize)
				.and_then([&]() { return serializer.Deserialize(buffer, offset, readVersion); })
				.and_then([&]() { return serializer.Deserialize(buffer, offset, readEntryCount); })
				.and_then([&]() { return serializer.Deserialize(buffer, offset, readTimestamp); });

			if (!res)
				return res;

			const zU32 expectedMinHeaderSize = static_cast<zU32>(m_Magic.size() + c_EnvelopeSize);
			if (readHeaderSize < expectedMinHeaderSize)
			{
				return UNEXPECTED("Повреждённый размер заголовка: {} байт (минимальный ожидаемый размер: {})",
					readHeaderSize, expectedMinHeaderSize);
			}

			// Если в *this уже задана ожидаемая версия — проверяем совместимость мажорной версии
			if (m_Version != Version{} && readVersion.GetMajor() != m_Version.GetMajor())
			{
				return UNEXPECTED("Несовместимая версия формата: {} (ожидалась мажорная версия {}).",
					readVersion.ToString(), m_Version.GetMajor());
			}

			m_HeaderSize = readHeaderSize;
			m_Version = readVersion;
			m_EntryCount = readEntryCount;
			m_Timestamp = readTimestamp;

			offset = startOffset + m_HeaderSize;
			return {};
		}

	private:
		Magic	m_Magic;
		zU32	m_HeaderSize;
		Version	m_Version;
		zU32	m_EntryCount;
		zU64	m_Timestamp;
	};
}
