#pragma once

#include "../../../header.h"
#include <core/serialize/Serializer.h>

namespace zzz::engine
{
	using namespace zzz::common;
	class ConfigiOS final : public ISerializable
	{
	public:
		ConfigiOS();
		~ConfigiOS() override = default;

	private:
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const Serializer& s) const override;
		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& s) override;
	};
}



