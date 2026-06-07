#include "pch.h"

#include "headers/constants.h"
#include "EngineConfig.h"

using namespace zzz::engine;

EngineConfig::EngineConfig(std::shared_ptr<IConfig> platformConfig) :
	m_Version(c_ConfigFileMajorVersion, c_ConfigFileMinorVersion, c_ConfigFilePatchVersion),
	m_PlatformConfig(std::move(platformConfig))
{
	ensure(m_PlatformConfig != nullptr, "Platform config must not be null.");
}

[[nodiscard]] std::expected<void, std::string> EngineConfig::Serialize(std::vector<std::byte>& buffer, const Serializer& s) const
{
	return s.Serialize(buffer, c_ConfigHeader)
		.and_then([&]() { return s.Serialize(buffer, m_Version); })
		.and_then([&]() { return s.Serialize(buffer, *m_PlatformConfig); });
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
		.and_then([&]() { return s.DeSerialize(buffer, offset, *m_PlatformConfig); });
}