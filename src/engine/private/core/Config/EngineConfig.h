#pragma once

#include "../../../header.h"
#include "../Utils/version.h"
#include "../Serialize/serializer.h"

namespace zzz::engine
{
	class EngineConfig final : public ISerializable
	{
	public:
		EngineConfig();

	private:
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const Serializer& s) const override;
		[[nodiscard]] std::expected<void, std::string> DeSerialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& s) override;

		Version m_Version;
	};
}