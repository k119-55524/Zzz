#if Z_MACOS

#include "ConfigMacOS.h"

using namespace zzz::core;

ConfigMacOS::ConfigMacOS()
{
}

[[nodiscard]] std::expected<void, std::string> ConfigMacOS::Serialize([[maybe_unused]] std::vector<std::byte>& buffer, [[maybe_unused]] const Serializer& s) const
{
	return std::expected<void, std::string>{};
}

[[nodiscard]] std::expected<void, std::string> ConfigMacOS::Deserialize([[maybe_unused]] std::span<const std::byte> buffer, [[maybe_unused]] std::size_t& offset, [[maybe_unused]] const Serializer& s)
{
	return std::expected<void, std::string>{};
}

#endif // Z_MACOS
