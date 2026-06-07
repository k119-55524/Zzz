#if defined(Z_ANDROID)

#include "ConfigAndroid.h"

using namespace zzz::engine;

ConfigAndroid::ConfigAndroid()
{
}

[[nodiscard]] std::expected<void, std::string> ConfigAndroid::Serialize(std::vector<std::byte>& buffer, const Serializer& s) const
{
	return std::expected<void, std::string>{};
}

[[nodiscard]] std::expected<void, std::string> ConfigAndroid::DeSerialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& s)
{
	return std::expected<void, std::string>{};
}

#endif // Z_ANDROID
