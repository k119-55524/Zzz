
#include "EngineConfig.h"
#include "headers/constants.h"

using namespace zzz::engine;

EngineConfig::EngineConfig() :
	m_Version(c_ConfigFileMajorVersion, c_ConfigFileMinorVersion, c_ConfigFilePatchVersion)
{
}

[[nodiscard]] std::expected<void, std::string> EngineConfig::Serialize(std::vector<std::byte>& buffer, const Serializer& s) const
{
	return s.Serialize(buffer, c_ConfigHeader)
		.and_then([&]() { return s.Serialize(buffer, m_Version); })
		.and_then([&]() { return s.Serialize(buffer, m_PlatformConfig); });
}

[[nodiscard]] std::expected<void, std::string> EngineConfig::DeSerialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& s)
{
	std::array<std::byte, c_ConfigHeader.size()> header;

	return s.DeSerialize(buffer, offset, header)
		.and_then([&]() -> std::expected<void, std::string>
			{
				if (header != c_ConfigHeader)
					return std::unexpected("Invalid config header.");

				return s.DeSerialize(buffer, offset, m_Version);
			})
		.and_then([&]() { return s.DeSerialize(buffer, offset, m_PlatformConfig); });
}