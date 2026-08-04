#include "ConfigEditor.h"

using namespace zzz::core;

ConfigEditor::ConfigEditor()
{
}

[[nodiscard]] std::expected<void, std::string> ConfigEditor::Serialize([[maybe_unused]] std::vector<std::byte>& buffer, [[maybe_unused]] const Serializer& s) const
{
	return {};
}

[[nodiscard]] std::expected<void, std::string> ConfigEditor::Deserialize([[maybe_unused]] std::span<const std::byte> buffer, [[maybe_unused]] std::size_t& offset, [[maybe_unused]] const Serializer& s)
{
	return {};
}
