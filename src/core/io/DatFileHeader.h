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
#include <type_traits>

#include "core/utils/Version.h"
#include "core/logger/logger.h"
#include "core/serialize/Serializer.h"
#include "core/utils/macros/MiscMacros.h"
#include "core/constants/LogCategoryConstants.h"
#include "core/constants/PackagesConstants.h"

namespace zzz::core
{
	/**
	 * @brief Дополнительные метаданные заголовка архива data.dat (12 байт).
	 */
	struct DataDatHeaderExtra
	{
		zU32 packCount = 0;     ///< Количество внешних паков в Таблице 1
		zU32 inlineCount = 0;   ///< Количество записей в Таблице 2 (inline data.dat)
		zU32 externalCount = 0; ///< Количество записей в Таблице 3 (внешние .dat пакеты)

		[[nodiscard]] static constexpr zU32 BinarySize() noexcept { return 12; }

		[[nodiscard]] constexpr auto operator<=>(const DataDatHeaderExtra&) const noexcept = default;

		template <typename SerializerType>
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const SerializerType& serializer) const
		{
			return serializer.Serialize(buffer, packCount)
				.and_then([&]() { return serializer.Serialize(buffer, inlineCount); })
				.and_then([&]() { return serializer.Serialize(buffer, externalCount); });
		}

		template <typename SerializerType>
		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const SerializerType& serializer)
		{
			return serializer.Deserialize(buffer, offset, packCount)
				.and_then([&]() { return serializer.Deserialize(buffer, offset, inlineCount); })
				.and_then([&]() { return serializer.Deserialize(buffer, offset, externalCount); });
		}
	};

	namespace detail
	{
		template <typename T>
		struct ExtraTraits
		{
			static constexpr bool HasExtra = true;
			static constexpr zU32 SerializedSize = T::BinarySize();
		};

		template <>
		struct ExtraTraits<void>
		{
			static constexpr bool HasExtra = false;
			static constexpr zU32 SerializedSize = 0;
		};

		struct EmptyExtra
		{
			[[nodiscard]] constexpr auto operator<=>(const EmptyExtra&) const noexcept = default;
		};
	}

	/**
	 * @brief Обобщённый заголовок бинарного пакета движка с поддержкой Extra и автовыравнивания.
	 *
	 * Фиксированные дисковые варианты:
	 * - PackageDatHeader (DatFileHeader): без Extra, Alignment = 1, размер 31 байт;
	 * - DataDatHeader: DataDatHeaderExtra (12 байт), Alignment = 64, размер 64 байта;
	 * - PakFileHeader: без Extra, Alignment = 4096, размер 4096 байт.
	 */
	template <typename TExtra = void, zU32 Alignment = 1>
	class GenericDatFileHeader final : public ISerializable
	{
	public:
		using Magic = DatMagic;
		using ExtraType = TExtra;

		/// @brief Размер конверта заголовка без сигнатуры (4 + 12 + 4 + 8 = 28 байт).
		static constexpr zU32 c_EnvelopeSize = static_cast<zU32>(sizeof(zU32) + Version::BinarySize() + sizeof(zU32) + sizeof(zU64));

		/// @brief Базовый размер заголовка по умолчанию для стандартных 3-байтовых сигнатур (31 байт).
		static constexpr zU32 c_BaseHeaderSize = 3 + c_EnvelopeSize;

		/// @brief Размер блока дополнительных данных заголовка.
		static constexpr zU32 c_ExtraDataSize = detail::ExtraTraits<TExtra>::SerializedSize;

		/// @brief Размер заголовка до применения выравнивания.
		static constexpr zU32 c_UnpaddedSize = c_BaseHeaderSize + c_ExtraDataSize;

		/// @brief Требуемое выравнивание заголовка на диске.
		static constexpr zU32 c_Alignment = Alignment;

		/// @brief Размер нулевого padding для достижения c_Alignment.
		static constexpr zU32 c_PaddingSize = (c_Alignment - (c_UnpaddedSize % c_Alignment)) % c_Alignment;

		/// @brief Полный размер заголовка на диске с учётом Extra и padding.
		static constexpr zU32 c_FullHeaderSize = c_UnpaddedSize + c_PaddingSize;

		/// @brief Конструктор по умолчанию.
		constexpr GenericDatFileHeader() noexcept = default;

		/// @brief Конструктор для заголовков без Extra (PackageDatHeader, PakFileHeader).
		template <typename E = TExtra>
			requires std::is_void_v<E>
		constexpr GenericDatFileHeader(
			const DatFileFormat& format,
			zU32 entryCount = 0,
			zU64 timestamp = 0) noexcept :
			m_Magic{ format.Magic },
			m_HeaderSize{ c_FullHeaderSize },
			m_Version{ format.FormatVersion },
			m_EntryCount{ entryCount },
			m_Timestamp{ timestamp }
		{}

		template <typename E = TExtra>
			requires std::is_void_v<E>
		constexpr GenericDatFileHeader(
			const Magic& magic,
			const Version& version,
			zU32 entryCount = 0,
			zU64 timestamp = 0) noexcept :
			m_Magic{ magic },
			m_HeaderSize{ c_FullHeaderSize },
			m_Version{ version },
			m_EntryCount{ entryCount },
			m_Timestamp{ timestamp }
		{}

		/// @brief Конструктор для заголовков с Extra (DataDatHeader).
		template <typename E = TExtra>
			requires (!std::is_void_v<E>)
		constexpr GenericDatFileHeader(
			const DatFileFormat& format,
			zU32 entryCount = 0,
			zU64 timestamp = 0,
			const E& extra = {}) noexcept :
			m_Magic{ format.Magic },
			m_HeaderSize{ c_FullHeaderSize },
			m_Version{ format.FormatVersion },
			m_EntryCount{ entryCount },
			m_Timestamp{ timestamp },
			m_Extra{ extra }
		{}

		template <typename E = TExtra>
			requires (!std::is_void_v<E>)
		constexpr GenericDatFileHeader(
			const Magic& magic,
			const Version& version,
			zU32 entryCount = 0,
			zU64 timestamp = 0,
			const E& extra = {}) noexcept :
			m_Magic{ magic },
			m_HeaderSize{ c_FullHeaderSize },
			m_Version{ version },
			m_EntryCount{ entryCount },
			m_Timestamp{ timestamp },
			m_Extra{ extra }
		{}

		[[nodiscard]] constexpr const Magic& GetMagic() const noexcept { return m_Magic; }
		[[nodiscard]] constexpr zU32 GetHeaderSize() const noexcept { return m_HeaderSize; }
		[[nodiscard]] constexpr const Version& GetVersion() const noexcept { return m_Version; }
		[[nodiscard]] constexpr zU32 GetEntryCount() const noexcept { return m_EntryCount; }
		[[nodiscard]] constexpr zU64 GetTimestamp() const noexcept { return m_Timestamp; }

		template <typename E = TExtra>
			requires (!std::is_void_v<E>)
		[[nodiscard]] constexpr const E& GetExtra() const noexcept { return m_Extra; }

		template <typename E = TExtra>
			requires (!std::is_void_v<E>)
		[[nodiscard]] constexpr E& GetExtra() noexcept { return m_Extra; }

		template <typename E = TExtra>
			requires (!std::is_void_v<E>)
		constexpr void SetExtra(const E& extra) noexcept { m_Extra = extra; }

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
			const std::size_t minExpectedSize = m_Magic.size() + c_EnvelopeSize + c_ExtraDataSize;
			if (headBytes.size() < minExpectedSize)
				return UNEXPECTED("Буфер заголовка меньше базового размера: {} < {} байт", headBytes.size(), minExpectedSize);

			Serializer serializer;
			std::size_t offset = 0;
			if (auto res = serializer.Deserialize(headBytes, offset, *this); !res)
				return UNEXPECTED("Ошибка десериализации заголовка: {}", res.error());

			if (static_cast<std::uintmax_t>(m_HeaderSize) > fileSize)
				return UNEXPECTED("Размер заголовка ({} байт) выходит за пределы размера файла ({} байт)", m_HeaderSize, fileSize);

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
			if constexpr (std::is_same_v<TExtra, DataDatHeaderExtra>)
			{
				DOut(Assets, "{}  packCount: {}", prefix, m_Extra.packCount);
				DOut(Assets, "{}  inlineCount: {}", prefix, m_Extra.inlineCount);
				DOut(Assets, "{}  externalCount: {}", prefix, m_Extra.externalCount);
			}
#endif // Z_ADD_LOGGER
		}

		[[nodiscard]] constexpr auto operator<=>(const GenericDatFileHeader& other) const noexcept
		{
			if (auto cmp = m_Magic <=> other.m_Magic; cmp != 0) return cmp;
			if (auto cmp = m_HeaderSize <=> other.m_HeaderSize; cmp != 0) return cmp;
			if (auto cmp = m_Version <=> other.m_Version; cmp != 0) return cmp;
			if (auto cmp = m_EntryCount <=> other.m_EntryCount; cmp != 0) return cmp;
			if (auto cmp = m_Timestamp <=> other.m_Timestamp; cmp != 0) return cmp;
			if constexpr (!std::is_void_v<TExtra>)
				return m_Extra <=> other.m_Extra;
			else
				return std::strong_ordering::equal;
		}
		[[nodiscard]] constexpr bool operator==(const GenericDatFileHeader& other) const noexcept
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

			auto res = serializer.Serialize(buffer, m_HeaderSize)
				.and_then([&]() { return serializer.Serialize(buffer, m_Version); })
				.and_then([&]() { return serializer.Serialize(buffer, m_EntryCount); })
				.and_then([&]() { return serializer.Serialize(buffer, m_Timestamp); });

			if (!res)
				return res;

			if constexpr (!std::is_void_v<TExtra>)
			{
				auto extraRes = m_Extra.Serialize(buffer, serializer);
				if (!extraRes)
					return extraRes;
			}

			if constexpr (c_PaddingSize > 0)
			{
				buffer.insert(buffer.end(), c_PaddingSize, std::byte{ 0 });
			}

			return {};
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

			const zU32 expectedMinHeaderSize = static_cast<zU32>(m_Magic.size() + c_EnvelopeSize + c_ExtraDataSize);
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

			if constexpr (!std::is_void_v<TExtra>)
			{
				auto extraRes = m_Extra.Deserialize(buffer, offset, serializer);
				if (!extraRes)
					return extraRes;
			}

			if (startOffset + readHeaderSize > buffer.size())
			{
				return UNEXPECTED("Размер заголовка ({} байт) выходит за пределы переданного буфера ({} байт)",
					readHeaderSize, buffer.size() - startOffset);
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
		[[no_unique_address]] std::conditional_t<std::is_void_v<TExtra>, detail::EmptyExtra, TExtra> m_Extra{};
	};

	using PackageDatHeader = GenericDatFileHeader<void, 1>;
	using DatFileHeader    = PackageDatHeader;
	using DataDatHeader    = GenericDatFileHeader<DataDatHeaderExtra, 64>;
	using PakFileHeader    = GenericDatFileHeader<void, 4096>;

	static_assert(PackageDatHeader::c_FullHeaderSize == 31, "PackageDatHeader must be 31 bytes");
	static_assert(DataDatHeader::c_FullHeaderSize == 64, "DataDatHeader must be 64 bytes");
	static_assert(PakFileHeader::c_FullHeaderSize == 4096, "PakFileHeader must be 4096 bytes");
	static_assert(PackageDatHeader::c_PaddingSize == 0, "PackageDatHeader padding must be 0");
	static_assert(DataDatHeader::c_PaddingSize == 21, "DataDatHeader padding must be 21 bytes");
	static_assert(PakFileHeader::c_PaddingSize == 4065, "PakFileHeader padding must be 4065 bytes");
}
