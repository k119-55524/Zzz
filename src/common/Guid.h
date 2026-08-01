#pragma once

#include <array>
#include <format>
#include <string>
#include <cstdint>
#include <common/serialize/Serializer.h>

namespace zzz::common
{
	/// @brief Представляет 128-битный глобальный уникальный идентификатор (GUID/UUID).
	/// @details Обеспечивает неиммутабельность хранения байтов, безопасную сериализацию и операторы сравнения C++20.
	class Guid final : public ISerializable
	{
	public:
		/// @brief Массив из 16 байт сырого представления GUID.
		using RawBytes = std::array<uint8_t, 16>;

		/// @brief Конструктор по умолчанию. Инициализирует нулевой GUID.
		constexpr Guid() noexcept : m_Bytes{} {}

		/// @brief Конструктор из сырого массива из 16 байт.
		/// @param bytes Исходный массив байт.
		constexpr explicit Guid(const RawBytes& bytes) noexcept : m_Bytes(bytes) {}

		/// @brief Возвращает постоянную ссылку на 16-байтовое представление GUID.
		/// @return Ссылка на константный массив байт.
		[[nodiscard]] constexpr const RawBytes& GetBytes() const noexcept { return m_Bytes; }

		/// @brief Проверяет, является ли GUID нулевым (пустым).
		/// @return true, если все 16 байт равны 0; иначе false.
		[[nodiscard]] constexpr bool IsEmpty() const noexcept
		{
			for (const auto b : m_Bytes)
				if (b != 0) return false;

			return true;
		}

		/// @brief Форматирует GUID в стандартную дефисную шестнадцатеричную строку (8-4-4-4-12).
		/// @return Форматированная строка GUID в нижнем регистре.
		[[nodiscard]] inline std::string ToString() const
		{
			return std::format("{:02x}{:02x}{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}",
				m_Bytes[0], m_Bytes[1], m_Bytes[2], m_Bytes[3],
				m_Bytes[4], m_Bytes[5],
				m_Bytes[6], m_Bytes[7],
				m_Bytes[8], m_Bytes[9],
				m_Bytes[10], m_Bytes[11], m_Bytes[12], m_Bytes[13], m_Bytes[14], m_Bytes[15]);
		}

		/// @brief Сравнение на равенство и неравенство.
		[[nodiscard]] constexpr bool operator==(const Guid& other) const noexcept
		{
			return m_Bytes == other.m_Bytes;
		}

		[[nodiscard]] constexpr bool operator!=(const Guid& other) const noexcept
		{
			return m_Bytes != other.m_Bytes;
		}

		/// @brief Трехстороннее сравнение C++20 (spaceship operator).
		[[nodiscard]] constexpr auto operator<=>(const Guid&) const noexcept = default;

	protected:
		/// @brief Сериализует 16 байт GUID в буфер.
		/// @param buffer Целевой вектор байт.
		/// @param s Ссылка на сериализатор.
		/// @return Результат сериализации.
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const Serializer& s) const override
		{
			return s.Serialize(buffer, std::span<const std::byte>(std::as_bytes(std::span(m_Bytes))));
		}

		/// @brief Десериализует 16 байт GUID из буфера.
		/// @param buffer Исходный буфер байт.
		/// @param offset Текущее смещение в буфере.
		/// @param s Ссылка на сериализатор.
		/// @return Результат десериализации.
		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& s) override
		{
			return s.Deserialize(buffer, offset, std::span<std::byte>(std::as_writable_bytes(std::span(m_Bytes))));
		}

	private:
		RawBytes m_Bytes;
	};
}

namespace std
{
	template <>
	struct hash<zzz::common::Guid>
	{
		size_t operator()(const zzz::common::Guid& guid) const noexcept
		{
			const auto& bytes = guid.GetBytes();
			uint64_t low = 0, high = 0;
			std::memcpy(&low, bytes.data(), sizeof(uint64_t));
			std::memcpy(&high, bytes.data() + sizeof(uint64_t), sizeof(uint64_t));
			return static_cast<size_t>(low ^ (high + 0x9e3779b97f4a7c15ULL + (low << 6) + (low >> 2)));
		}
	};
}
