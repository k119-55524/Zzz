#include "pch.h"
export module serializer;

import result;

namespace zzz
{
	export class serializer
	{
	public:
		serializer() = default;
		serializer(serializer&) = default;
		serializer(serializer&&) = default;

		serializer& operator=(const serializer&) = delete;
		serializer& operator=(serializer&&) noexcept = delete;

		// ѕримитивные типы
		template<typename T> requires std::is_trivially_copyable_v<T>
		void Serialize(std::vector<std::byte>& buffer, const T& value) const
		{
			auto bytes = std::bit_cast<std::array<std::byte, sizeof(T)>>(value);
			buffer.insert(buffer.end(), bytes.begin(), bytes.end());
		}

		template<typename T> requires std::is_trivially_copyable_v<T>
		result<> DeSerialize(std::span<const std::byte> buffer, std::size_t& offset, T& value) const
		{
			if (offset + sizeof(T) > buffer.size())
				return Unexpected(eResult::buffer_too_small, L"[zSerialize.DeSerialize]: Buffer too small.");

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

		result<> DeSerialize(std::span<const std::byte> buffer, std::size_t& offset, std::wstring& str) const
		{
			if (offset + sizeof(uint32_t) > buffer.size())
				return Unexpected(eResult::buffer_too_small, L"[zSerialize.DeSerialize]: Buffer too small for length.");

			uint32_t length;
			memcpy(&length, buffer.data() + offset, sizeof(length));
			offset += sizeof(length);

			size_t byteSize = length * sizeof(wchar_t);
			if (offset + byteSize > buffer.size())
				return Unexpected(eResult::buffer_too_small, L"[zSerialize.DeSerialize]: Buffer too small for string.");

			str.resize(length);
			memcpy(str.data(), buffer.data() + offset, byteSize);
			offset += byteSize;

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

		result<> DeSerialize(std::span<const std::byte> buffer, std::size_t& offset, std::string& str) const
		{
			if (offset + sizeof(uint32_t) > buffer.size())
				return Unexpected(eResult::buffer_too_small, L"[zSerialize.DeSerialize]: Buffer too small for string length.");

			uint32_t length;
			memcpy(&length, buffer.data() + offset, sizeof(length));
			offset += sizeof(length);

			if (offset + length > buffer.size())
				return Unexpected(eResult::buffer_too_small, L"[zSerialize.DeSerialize]: Buffer too small for string data.");

			str.resize(length);
			memcpy(str.data(), buffer.data() + offset, length);
			offset += length;

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
		result<> DeSerialize(std::span<const std::byte> buffer, std::size_t& offset, std::vector<T>& vec) const
		{
			uint32_t length;
			auto res = DeSerialize(buffer, offset, length);
			if (!res)
				return res;

			size_t byteSize = length * sizeof(T);
			if (offset + byteSize > buffer.size())
				return Unexpected(eResult::buffer_too_small, L"[zSerialize.DeSerialize]: Buffer too small for vector data.");

			vec.resize(length);
			memcpy(vec.data(), buffer.data() + offset, byteSize);
			offset += byteSize;
			return eResult::success;
		}
	};
}
