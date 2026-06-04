#include "ConfigMSWin.h"

using namespace zzz::engine;

ConfigMSWin::ConfigMSWin() :
	m_IcoResourceName("IDI_ICON1"),
	m_ClassName("ZzzEngineWindowClass")
{

}

[[nodiscard]] std::expected<void, std::string> ConfigMSWin::Serialize(std::vector<std::byte>& buffer, const Serializer& s) const
{
	return s.Serialize(buffer, m_IcoResourceName)
		.and_then([&](void) { return s.Serialize(buffer, m_ClassName); });
}

[[nodiscard]] std::expected<void, std::string> ConfigMSWin::DeSerialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& s)
{
	return s.DeSerialize(buffer, offset, m_IcoResourceName)
		.and_then([&](void) { return s.DeSerialize(buffer, offset, m_ClassName); });
}