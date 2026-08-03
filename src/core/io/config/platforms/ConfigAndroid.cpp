
#include "ConfigAndroid.h"

using namespace zzz::engine;

ConfigAndroid::ConfigAndroid()
{
}

[[nodiscard]] std::expected<void, std::string> ConfigAndroid::Serialize(std::vector<std::byte>& buffer, const Serializer& s) const
{
	return {};
}

[[nodiscard]] std::expected<void, std::string> ConfigAndroid::Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& s)
{
	return {};
}
