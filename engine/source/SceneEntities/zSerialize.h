#pragma once

#include "../Types/Types.h"

namespace Zzz
{
	class zSerialize
	{
	public:
		zSerialize() = default;
		zSerialize(zSerialize&) = default;
		zSerialize(zSerialize&&) = default;

		zSerialize& operator=(const zSerialize&) = default;
		zSerialize& operator=(zSerialize&&) noexcept = default;

		inline void Serialize(stringstream& buffer, const zStr& str) const
		{
			size_t textLength = str.size();
			buffer.write(reinterpret_cast<const char*>(&textLength), sizeof(textLength));
			buffer.write(reinterpret_cast<const char*>(str.data()), textLength * sizeof(wchar_t));
		}
		inline void DeSerialize(istringstream& buffer, zStr& str) const
		{
			size_t nameLength;
			buffer.read(reinterpret_cast<char*>(&nameLength), sizeof(nameLength));
			str.resize(nameLength);
			buffer.read(reinterpret_cast<char*>(str.data()), nameLength * sizeof(wchar_t));
		}
		template<typename T>
		inline void Serialize(stringstream& buffer, const T& value) const
		{
			buffer.write(reinterpret_cast<const char*>(&value), sizeof(value));
		}
		template<typename T>
		inline void DeSerialize(istringstream& buffer, T& value) const
		{
			buffer.read(reinterpret_cast<char*>(&value), sizeof(value));
		}
	};
}
