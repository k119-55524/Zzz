
#include "ConfigMSWin.h"

using namespace zzz::engine;

ConfigMSWin::ConfigMSWin()
{
}

[[nodiscard]] std::expected<void, std::string> ConfigMSWin::Serialize([[maybe_unused]] std::vector<std::byte>& buffer, [[maybe_unused]] const Serializer& s) const
{
	return {};
}

[[nodiscard]] std::expected<void, std::string> ConfigMSWin::Deserialize([[maybe_unused]] std::span<const std::byte> buffer, [[maybe_unused]] std::size_t& offset, [[maybe_unused]] const Serializer& s)
{
	return {};
}
