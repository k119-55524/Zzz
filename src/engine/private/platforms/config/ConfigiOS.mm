#if Z_IOS

#include "ConfigiOS.h"

using namespace zzz::engine;

ConfigiOS::ConfigiOS()
{
}

[[nodiscard]] std::expected<void, std::string> ConfigiOS::Serialize(std::vector<std::byte>& buffer, const Serializer& s) const
{
	return std::expected<void, std::string>{};
}

[[nodiscard]] std::expected<void, std::string> ConfigiOS::DeSerialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& s)
{
	return std::expected<void, std::string>{};
}

#endif // Z_IOS
