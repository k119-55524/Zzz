#if defined(Z_MACOS)

#include "ConfigMacOS.h"

using namespace zzz::engine;

ConfigMacOS::ConfigMacOS()
{
}

[[nodiscard]] std::expected<void, std::string> ConfigMacOS::Serialize(std::vector<std::byte>& buffer, const Serializer& s) const
{
	return std::expected<void, std::string>{};
}

[[nodiscard]] std::expected<void, std::string> ConfigMacOS::DeSerialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& s)
{
	return std::expected<void, std::string>{};
}

#endif // defined(Z_MACOS)
