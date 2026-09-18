#include "core/io/package/ShaderData.h"

namespace zzz::core
{
	ShaderData::ShaderData(std::string name)
		: m_Name(std::move(name))
	{
	}

	std::expected<void, std::string> ShaderData::Serialize(std::vector<std::byte>& buffer, const Serializer& serializer) const
	{
		return serializer.Serialize(buffer, m_Name);
	}

	std::expected<void, std::string> ShaderData::Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& serializer)
	{
		return serializer.Deserialize(buffer, offset, m_Name);
	}
}
