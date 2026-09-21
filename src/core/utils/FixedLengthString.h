#pragma once

#include <span>
#include <array>
#include <string>
#include <vector>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <expected>
#include <string_view>
#include <type_traits>

#include "core/utils/Ensure.h"
#include "core/logger/logger.h"
#include "core/serialize/Serializer.h"

namespace zzz::core
{
	namespace utf
	{
		[[nodiscard]] inline std::expected<std::u32string, std::string> Utf8ToUtf32(std::string_view utf8)
		{
			std::u32string result;
			result.reserve(utf8.size());
			const auto* p = reinterpret_cast<const uint8_t*>(utf8.data());
			const auto* end = p + utf8.size();

			while (p < end)
			{
				uint32_t cp = 0;
				uint8_t b0 = *p++;
				if ((b0 & 0x80) == 0)
				{
					cp = b0;
				}
				else if ((b0 & 0xE0) == 0xC0)
				{
					if (p >= end || (*p & 0xC0) != 0x80) return UNEXPECTED("Некорректная последовательность UTF-8");
					cp = ((b0 & 0x1F) << 6) | (*p++ & 0x3F);
					if (cp < 0x80) return UNEXPECTED("Overlong последовательность UTF-8 (2 байта)");
				}
				else if ((b0 & 0xF0) == 0xE0)
				{
					if (p + 1 >= end || (*p & 0xC0) != 0x80 || (*(p + 1) & 0xC0) != 0x80) return UNEXPECTED("Некорректная последовательность UTF-8");
					cp = ((b0 & 0x0F) << 12) | ((*p & 0x3F) << 6) | (*(p + 1) & 0x3F);
					p += 2;
					if (cp < 0x800) return UNEXPECTED("Overlong последовательность UTF-8 (3 байта)");
				}
				else if ((b0 & 0xF8) == 0xF0)
				{
					if (p + 2 >= end || (*p & 0xC0) != 0x80 || (*(p + 1) & 0xC0) != 0x80 || (*(p + 2) & 0xC0) != 0x80) return UNEXPECTED("Некорректная последовательность UTF-8");
					cp = ((b0 & 0x07) << 18) | ((*p & 0x3F) << 12) | ((*(p + 1) & 0x3F) << 6) | (*(p + 2) & 0x3F);
					p += 3;
					if (cp < 0x10000) return UNEXPECTED("Overlong последовательность UTF-8 (4 байта)");
				}
				else
				{
					return UNEXPECTED("Недопустимый начальный байт UTF-8");
				}

				if (cp >= 0xD800 && cp <= 0xDFFF)
				{
					return UNEXPECTED("Недопустимый кодовый пункт Unicode (суррогатный диапазон 0xD800-0xDFFF)");
				}
				if (cp > 0x10FFFF)
				{
					return UNEXPECTED("Кодовый пункт Unicode превышает допустимый максимум 0x10FFFF");
				}

				result.push_back(static_cast<char32_t>(cp));
			}
			return result;
		}

		[[nodiscard]] inline std::string Utf32ToUtf8(std::u32string_view utf32)
		{
			std::string result;
			result.reserve(utf32.size() * 3);
			for (char32_t cp : utf32)
			{
				if (cp == 0) break;
				auto val = static_cast<uint32_t>(cp);
				if (val <= 0x7F)
				{
					result.push_back(static_cast<char>(val));
				}
				else if (val <= 0x7FF)
				{
					result.push_back(static_cast<char>(0xC0 | (val >> 6)));
					result.push_back(static_cast<char>(0x80 | (val & 0x3F)));
				}
				else if (val <= 0xFFFF)
				{
					result.push_back(static_cast<char>(0xE0 | (val >> 12)));
					result.push_back(static_cast<char>(0x80 | ((val >> 6) & 0x3F)));
					result.push_back(static_cast<char>(0x80 | (val & 0x3F)));
				}
				else if (val <= 0x10FFFF)
				{
					result.push_back(static_cast<char>(0xF0 | (val >> 18)));
					result.push_back(static_cast<char>(0x80 | ((val >> 12) & 0x3F)));
					result.push_back(static_cast<char>(0x80 | ((val >> 6) & 0x3F)));
					result.push_back(static_cast<char>(0x80 | (val & 0x3F)));
				}
			}
			return result;
		}
	}

	/**
	 * @class FixedLengthString32
	 * @brief Фиксированная по числу символов UTF-32 строка для бинарных форматов движка (PackageEntry).
	 * @tparam MaxChars Максимальное количество символов, включая завершающий нулевой символ.
	 *
	 * @details Хранится как непрерывный массив std::array<char32_t, MaxChars>.
	 * Любой символ (латиница, кириллица, иероглифы, эмодзи) занимает ровно 1 слот (4 байта в UTF-32).
	 * Неиспользованный хвост буфера всегда зануляется.
	 * При сериализации и десериализации через Serializer передаётся строго ровно MaxChars * sizeof(char32_t) байт без префикса длины.
	 */
	template<std::size_t MaxChars>
	class FixedLengthString32 final : public ISerializable
	{
		static_assert(MaxChars > 0, "FixedLengthString32 MaxChars must be greater than 0.");

	public:
		using CharT = char32_t;

		constexpr FixedLengthString32() noexcept = default;

		[[nodiscard]] static std::expected<FixedLengthString32<MaxChars>, std::string> Create(std::string_view str) noexcept
		{
			auto u32Res = utf::Utf8ToUtf32(str);
			if (!u32Res)
			{
				return UNEXPECTED("Некорректная UTF-8 строка: {}", u32Res.error());
			}

			const auto& u32 = *u32Res;
			if (u32.size() >= MaxChars)
			{
				return UNEXPECTED("Число символов в имени '{}' ({}) превышает лимит (максимум {} символов с учётом null-терминатора).",
					str, u32.size(), MaxChars - 1);
			}

			FixedLengthString32<MaxChars> result;
			for (std::size_t i = 0; i < u32.size(); ++i)
			{
				result.m_Data[i] = u32[i];
			}
			result.m_Data[u32.size()] = CharT{ 0 };
			return result;
		}

		[[nodiscard]] constexpr static std::size_t CharacterCapacity() noexcept { return MaxChars; }
		[[nodiscard]] constexpr static std::size_t CapacityBytes() noexcept { return MaxChars * sizeof(CharT); }
		[[nodiscard]] constexpr static std::size_t BinarySize() noexcept { return CapacityBytes(); }

		[[nodiscard]] std::string ToString() const
		{
			std::size_t len = 0;
			while (len < MaxChars && m_Data[len] != CharT{ 0 })
			{
				++len;
			}
			return utf::Utf32ToUtf8(std::u32string_view(m_Data.data(), len));
		}

		[[nodiscard]] bool empty() const noexcept
		{
			return m_Data[0] == CharT{ 0 };
		}

		[[nodiscard]] std::size_t length() const noexcept
		{
			std::size_t len = 0;
			while (len < MaxChars && m_Data[len] != CharT{ 0 })
			{
				++len;
			}
			return len;
		}

		[[nodiscard]] const std::array<CharT, MaxChars>& GetRawBuffer() const noexcept
		{
			return m_Data;
		}

		[[nodiscard]] bool operator==(const FixedLengthString32<MaxChars>& other) const noexcept
		{
			return m_Data == other.m_Data;
		}

		[[nodiscard]] bool operator==(std::string_view other) const
		{
			return ToString() == other;
		}

		[[nodiscard]] bool operator==(const std::string& other) const
		{
			return ToString() == other;
		}

		[[nodiscard]] bool operator==(const char* other) const
		{
			return ToString() == other;
		}

	protected:
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const Serializer& serializer) const override
		{
			return serializer.Serialize(buffer, std::as_bytes(std::span(m_Data)));
		}

		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& serializer) override
		{
			auto res = serializer.Deserialize(buffer, offset, std::as_writable_bytes(std::span(m_Data)));
			if (!res)
			{
				return res;
			}
			m_Data[MaxChars - 1] = CharT{ 0 };
			return {};
		}

	private:
		std::array<CharT, MaxChars> m_Data{};
	};

	template<std::size_t MaxChars>
	using FixedLengthString = FixedLengthString32<MaxChars>;
}
