
#include "EngineConfig.h"
#include <common/constants.h>

using namespace zzz::io;
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

[[nodiscard]] std::expected<void, std::string> EngineConfig::Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& s)
{
	zFileHeader<3> header{};

	return s.Deserialize(buffer, offset, header)
		.and_then([&]() -> std::expected<void, std::string>
			{
				if (header != c_ConfigHeader)
					return UNEXPECTED("Некорректный заголовок конфигурации.");

				return s.Deserialize(buffer, offset, m_Version);
			})
		.and_then([&]() { return s.Deserialize(buffer, offset, m_PlatformConfig); });
}
