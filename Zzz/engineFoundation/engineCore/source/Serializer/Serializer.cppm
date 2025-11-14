
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
		virtual Result<> DeSerialize(std::span<const std::byte> buffer, const Serializer& serializer) = 0;

		friend class Serializer;
	};

	export class Serializer
	{
	public:
		Serializer() = default;
		Serializer(Serializer&) = default;
		Serializer(Serializer&&) = default;

		Serializer& operator=(const Serializer&) = delete;
		Serializer& operator=(Serializer&&) noexcept = delete;

		// Сериализация ISerializable объектов
		Result<> Serialize(std::vector<std::byte>& buffer, const ISerializable& obj) const
		{
			return obj.Serialize(buffer, *this);
		}

		Result<> DeSerialize(std::span<const std::byte> buffer, ISerializable& obj) const
		{
			return obj.DeSerialize(buffer, *this);
		}

		// Примитивные типы
		template<typename T> requires std::is_trivially_copyable_v<T>
		void Serialize(std::vector<std::byte>& buffer, const T& value) const
		{
			auto bytes = std::bit_cast<std::array<std::byte, sizeof(T)>>(value);
			buffer.insert(buffer.end(), bytes.begin(), bytes.end());
		}

		template<typename T> requires std::is_trivially_copyable_v<T>
		Result<> DeSerialize(std::span<const std::byte> buffer, std::size_t& offset, T& value) const
		{
			if (offset + sizeof(T) > buffer.size())
				return Unexpected(eResult::buffer_too_small, L"[Serializer.DeSerialize]: Buffer too small.");

			memcpy(&value, buffer.data() + offset, sizeof(T));
			offset += sizeof(T);

			return eResult::success;
		}

		// std::wstring
		void Serialize(std::vector<std::byte>& buffer, const std::wstring& str) const
		{
			uint32_t length = static_cast<uint32_t>(str.size());
			size_t offset = buffer.size();
			buffer.resize(offset + sizeof(length) + length * sizeof(wchar_t));
			memcpy(buffer.data() + offset, &length, sizeof(length));
			memcpy(buffer.data() + offset + sizeof(length), str.data(), length * sizeof(wchar_t));
		}

		Result<> DeSerialize(std::span<const std::byte> buffer, std::wstring& str) const
		{
			if (sizeof(uint32_t) > buffer.size())
				return Unexpected(eResult::buffer_too_small, L"[Serializer.DeSerialize]: Buffer too small for length.");

			uint32_t length;
			memcpy(&length, buffer.data(), sizeof(length));

			size_t byteSize = length * sizeof(wchar_t);
			if (byteSize > buffer.size())
				return Unexpected(eResult::buffer_too_small, L"[Serializer.DeSerialize]: Buffer too small for string.");

			str.resize(length);
			memcpy(str.data(), buffer.data(), byteSize);

			return {};
		}

		// std::string
		void Serialize(std::vector<std::byte>& buffer, const std::string& str) const
		{
			uint32_t length = static_cast<uint32_t>(str.size());
			size_t offset = buffer.size();
			buffer.resize(offset + sizeof(length) + length);
			memcpy(buffer.data() + offset, &length, sizeof(length));
			memcpy(buffer.data() + offset + sizeof(length), str.data(), length);
		}

		Result<> DeSerialize(std::span<const std::byte> buffer, std::string& str) const
		{
			if (sizeof(uint32_t) > buffer.size())
				return Unexpected(eResult::buffer_too_small, L"[Serializer.DeSerialize]: Buffer too small for string length.");

			uint32_t length;
			memcpy(&length, buffer.data(), sizeof(length));

			if (length > buffer.size())
				return Unexpected(eResult::buffer_too_small, L"[Serializer.DeSerialize]: Buffer too small for string data.");

			str.resize(length);
			memcpy(str.data(), buffer.data(), length);

			return {};
		}

		// std::vector<T> (только тривиальные типы)
		template<typename T> requires std::is_trivially_copyable_v<T>
		void Serialize(std::vector<std::byte>& buffer, const std::vector<T>& vec) const
		{
			uint32_t length = static_cast<uint32_t>(vec.size());
			Serialize(buffer, length);
			if (!vec.empty())
			{
				auto data = reinterpret_cast<const std::byte*>(vec.data());
				buffer.insert(buffer.end(), data, data + vec.size() * sizeof(T));
			}
		}

		template<typename T> requires std::is_trivially_copyable_v<T>
		Result<> DeSerialize(std::span<const std::byte> buffer, std::vector<T>& vec) const
		{
			uint32_t length;
			auto res = DeSerialize(buffer, length);
			if (!res)
				return res;

			size_t byteSize = length * sizeof(T);
			if (byteSize > buffer.size())
				return Unexpected(eResult::buffer_too_small, L"[Serializer.DeSerialize]: Buffer too small for vector data.");

			vec.resize(length);
			memcpy(vec.data(), buffer.data(), byteSize);

			return {};
		}
	};
}