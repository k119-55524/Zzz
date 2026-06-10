
#include "version.h"

using namespace zzz::engine;

[[nodiscard]] std::expected<void, std::string> Version::Serialize(std::vector<std::byte>& buffer, const Serializer& s) const
{
	return s.Serialize(buffer, m_Major)
		.and_then([&]() { return s.Serialize(buffer, m_Minor); })
		.and_then([&]() { return s.Serialize(buffer, m_Patch); });
}

[[nodiscard]] std::expected<void, std::string> Version::DeSerialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& s)
{
	return s.DeSerialize(buffer, offset, m_Major)
		.and_then([&]() { return s.DeSerialize(buffer, offset, m_Minor); })
		.and_then([&]() { return s.DeSerialize(buffer, offset, m_Patch); });
}