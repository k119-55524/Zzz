
#include "ConfigLinux.h"

using namespace zzz::core;

ConfigLinux::ConfigLinux()
{
}

[[nodiscard]] std::expected<void, std::string> ConfigLinux::Serialize([[maybe_unused]] std::vector<std::byte>& buffer, [[maybe_unused]] const Serializer& s) const
{
	return {};
}

[[nodiscard]] std::expected<void, std::string> ConfigLinux::Deserialize([[maybe_unused]] std::span<const std::byte> buffer, [[maybe_unused]] std::size_t& offset, [[maybe_unused]] const Serializer& s)
{
	return {};
}
