
#include "pch.h"

export module Serializer;

import Result;

namespace zzz
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

	export class Serializer
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
		template<typename T> requires std::is_arithmetic_v<T>
		Result<> DeSerialize(std::span<const std::byte> buffer, std::size_t& offset, T& value) const
		{
			if (offset + sizeof(T) > buffer.size())
				return Unexpected(eResult::buffer_too_small, L"[zSerialize.DeSerialize]: Buffer too small.");
			std::memcpy(&value, buffer.data() + offset, sizeof(T));
			offset += sizeof(T);

			return {};
		}

		// Сериализация std::string
		Result<> Serialize(std::vector<std::byte>& buffer, const std::string& str) const
		{
			// Сначала записываем размер строки
			const std::size_t size = str.size();
			auto res = Serialize(buffer, size);
			if (!res)
				return res;

			// Затем записываем данные строки
			const std::size_t old_size = buffer.size();
			buffer.resize(old_size + size);
			std::memcpy(buffer.data() + old_size, str.data(), size);

			return {};
		}

		// Десериализация std::string
		Result<> DeSerialize(std::span<const std::byte> buffer, std::size_t& offset, std::string& str) const
		{
			// Сначала читаем размер строки
			std::size_t size = 0;
			auto res = DeSerialize(buffer, offset, size);
			if (!res)
				return res;

			// Проверяем, достаточно ли данных в буфере
			if (offset + size > buffer.size())
				return Unexpected(eResult::buffer_too_small, L"[zSerialize.DeSerialize]: Buffer too small for string data.");

			// Читаем данные строки
			str.resize(size);
			std::memcpy(str.data(), buffer.data() + offset, size);
			offset += size;

			return {};
		}

		// Сериализация std::wstring
		Result<> Serialize(std::vector<std::byte>& buffer, const std::wstring& str) const
		{
			// Сначала записываем размер строки
			const std::size_t size = str.size();
			auto res = Serialize(buffer, size);
			if (!res)
				return res;

			// Затем записываем данные строки (размер в байтах)
			const std::size_t byte_size = size * sizeof(wchar_t);
			const std::size_t old_size = buffer.size();
			buffer.resize(old_size + byte_size);
			std::memcpy(buffer.data() + old_size, str.data(), byte_size);

			return {};
		}

		// Десериализация std::wstring
		Result<> DeSerialize(std::span<const std::byte> buffer, std::size_t& offset, std::wstring& str) const
		{
			// Сначала читаем размер строки (количество символов)
			std::size_t size = 0;
			auto res = DeSerialize(buffer, offset, size);
			if (!res)
				return res;

			// Вычисляем размер в байтах
			const std::size_t byte_size = size * sizeof(wchar_t);

			// Проверяем, достаточно ли данных в буфере
			if (offset + byte_size > buffer.size())
				return Unexpected(eResult::buffer_too_small, L"[zSerialize.DeSerialize]: Buffer too small for wstring data.");

			// Читаем данные строки
			str.resize(size);
			std::memcpy(str.data(), buffer.data() + offset, byte_size);
			offset += byte_size;

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