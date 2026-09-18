#include "core/io/package/MaterialData.h"

namespace zzz::core
{
	MaterialData::MaterialData(std::string name, Guid shaderGuid)
		: m_Name(std::move(name))
		, m_ShaderGuid(shaderGuid)
	{
	}

	std::expected<void, std::string> MaterialData::Serialize(std::vector<std::byte>& buffer, const Serializer& serializer) const
	{
		auto res = serializer.Serialize(buffer, m_Name);
		if (!res) return res;
		return serializer.Serialize(buffer, m_ShaderGuid);
	}

	std::expected<void, std::string> MaterialData::Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& serializer)
	{
		auto res = serializer.Deserialize(buffer, offset, m_Name);
		if (!res) return res;
		return serializer.Deserialize(buffer, offset, m_ShaderGuid);
	}
}
