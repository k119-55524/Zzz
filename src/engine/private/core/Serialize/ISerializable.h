#pragma once

#include <span>

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

		friend class Serializer;
	};
}