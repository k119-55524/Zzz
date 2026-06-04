#include "pch.h"

#include "headers/constants.h"
#include "EngineConfig.h"

using namespace zzz::engine;

EngineConfig::EngineConfig() :
	m_Version(configFileMajorVersion, configFileMinorVersion, configFilePatchVersion),
	m_WinSize(800, 600)
{
}

[[nodiscard]] std::expected<void, std::string> EngineConfig::Serialize(std::vector<std::byte>& buffer, const Serializer& s) const
{
	return s.Serialize(buffer, configHeader)
		.and_then([&]() { return s.Serialize(buffer, m_Version); })
		.and_then([&]() { return s.Serialize(buffer, m_PlatformConfig); });
}

[[nodiscard]] std::expected<void, std::string> EngineConfig::DeSerialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& s)
{
	std::array<std::byte, configHeader.size()> header;

	return s.DeSerialize(buffer, offset, header)
		.and_then([&]() -> std::expected<void, std::string>
			{
				if (header != configHeader)
					return std::unexpected("Invalid config header.");

				return s.DeSerialize(buffer, offset, m_Version);
			})
		.and_then([&]() { return s.DeSerialize(buffer, offset, m_PlatformConfig); });
}