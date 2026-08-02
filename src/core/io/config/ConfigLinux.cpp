
#include "ConfigLinux.h"

using namespace zzz::engine;

ConfigLinux::ConfigLinux()
{
}

[[nodiscard]] std::expected<void, std::string> ConfigLinux::Serialize(std::vector<std::byte>& buffer, const Serializer& s) const
{
	return std::expected<void, std::string>{};
}

[[nodiscard]] std::expected<void, std::string> ConfigLinux::Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& s)
{
	return std::expected<void, std::string>{};
}

