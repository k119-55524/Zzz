#pragma once

#include <span>
#include <vector>
#include <string>
#include <expected>

namespace zzz::engine
{
	class Serializer;

	class ISerializable
	{
	public:
		virtual ~ISerializable() = default;

	protected:
		[[nodiscard]] virtual std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const Serializer& serializer) const = 0;
		[[nodiscard]] virtual std::expected<void, std::string> DeSerialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& serializer) = 0;

		friend class zzz::engine::Serializer;
	};

	class Serializer
	{
	public:
		Serializer() = default;
		Serializer(Serializer&) = default;
		Serializer(Serializer&&) = default;
		Serializer& operator=(const Serializer&) = default;
		Serializer& operator=(Serializer&&) noexcept = default;

		// Сериализация примитивных типов
		template<typename T> requires std::is_arithmetic_v<T>
		std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const T& value) const
		{
			const std::size_t old_size = buffer.size();
			buffer.resize(old_size + sizeof(T));
			std::memcpy(buffer.data() + old_size, &value, sizeof(T));

			return {};
		}

		// Десериализация примитивных типов
		template<typename T> requires std::is_arithmetic_v<T>
		std::expected<void, std::string> DeSerialize(std::span<const std::byte> buffer, std::size_t& offset, T& value) const
		{
			if (offset + sizeof(T) > buffer.size())
				UNEXPECTED("Buffer too small.");
			std::memcpy(&value, buffer.data() + offset, sizeof(T));
			offset += sizeof(T);

			return {};
		}

		// Сериализация std::array<std::byte, N>
		template<std::size_t N>
		std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const std::array<std::byte, N>& value) const
		{
			const auto oldSize = buffer.size();
			buffer.resize(oldSize + N);
			std::memcpy(buffer.data() + oldSize, value.data(), N);

			return {};
		}

		// Десериализация std::array<std::byte, N>
		template<std::size_t N>
		std::expected<void, std::string> DeSerialize(std::span<const std::byte> buffer, std::size_t& offset, std::array<std::byte, N>& value) const
		{
			if (offset + N > buffer.size())
				UNEXPECTED("Buffer too small.");

			std::memcpy(value.data(), buffer.data() + offset, N);
			offset += N;

			return {};
		}

		// Сериализация std::string
		std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const std::string& str) const;
		// Десериализация std::string
		std::expected<void, std::string> DeSerialize(std::span<const std::byte> buffer, std::size_t& offset, std::string& str) const;
		// Сериализация std::wstring
		std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const std::wstring& str) const;
		// Десериализация std::wstring
		std::expected<void, std::string> DeSerialize(std::span<const std::byte> buffer, std::size_t& offset, std::wstring& str) const;
		// Сериализация объектов ISerializable
		std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const ISerializable& obj) const;
		// Десериализация объектов ISerializable
		std::expected<void, std::string> DeSerialize(std::span<const std::byte> buffer, std::size_t& offset, ISerializable& obj) const;
	};
}