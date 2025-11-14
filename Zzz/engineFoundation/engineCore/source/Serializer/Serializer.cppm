
#include "pch.h"

export module Serializer;

import Result;

namespace zzz::core
{
	export class Serializer;

	export class ISerializable
	{
	public:
		virtual ~ISerializable() = default;

	protected:
		virtual Result<> Serialize(std::vector<std::byte>& buffer, const Serializer& serializer) const = 0;
		virtual Result<> DeSerialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& serializer) = 0;

		friend class Serializer;
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
		Result<> Serialize(std::vector<std::byte>& buffer, const T& value) const
		{
			const std::size_t old_size = buffer.size();
			buffer.resize(old_size + sizeof(T));
			std::memcpy(buffer.data() + old_size, &value, sizeof(T));

			return {};
		}

		// Десериализация примитивных типов
		template<typename T>
			requires std::is_arithmetic_v<T>
		Result<> DeSerialize(std::span<const std::byte> buffer, std::size_t& offset, T& value) const
		{
			if (offset + sizeof(T) > buffer.size())
				Unexpected(eResult::buffer_too_small, L"[zSerialize.DeSerialize]: Buffer too small.");

			std::memcpy(&value, buffer.data() + offset, sizeof(T));
			offset += sizeof(T);

			return {};
		}

		// Сериализация объектов ISerializable
		Result<> Serialize(std::vector<std::byte>& buffer, const ISerializable& obj) const
		{
			return obj.Serialize(buffer, *this);
		}

		// Десериализация объектов ISerializable
		Result<> DeSerialize(std::span<const std::byte> buffer, std::size_t& offset, ISerializable& obj) const
		{
			return obj.DeSerialize(buffer, offset, *this);
		}
	};
}
