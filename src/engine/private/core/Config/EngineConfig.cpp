#include "EngineConfig.h"

using namespace zzz::engine;

EngineConfig::EngineConfig()
{
}

[[nodiscard]] std::expected<void, std::string> EngineConfig::Serialize(std::vector<std::byte>& buffer, const Serializer& s) const
{
	return s.Serialize(buffer, m_Version);
	//return s.Serialize(buffer, m_AppName)
	//	.and_then([&]() { return s.Serialize(buffer, m_ClassName); })
	//	.and_then([&]() { return s.Serialize(buffer, m_WinSize); })
	//	.and_then([&]() { return s.Serialize(buffer, m_IcoFullPath); })
	//	.and_then([&]() { return s.Serialize(buffer, m_IcoSize); });
}

[[nodiscard]] std::expected<void, std::string> EngineConfig::DeSerialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& s)
{
	return s.DeSerialize(buffer, offset, m_Version);
	//return s.DeSerialize(buffer, offset, m_AppName)
	//	.and_then([&]() { return s.DeSerialize(buffer, offset, m_ClassName); })
	//	.and_then([&]() { return s.DeSerialize(buffer, offset, m_WinSize); })
	//	.and_then([&]() { return s.DeSerialize(buffer, offset, m_IcoFullPath); })
	//	.and_then([&]() { return s.DeSerialize(buffer, offset, m_IcoSize); });
}