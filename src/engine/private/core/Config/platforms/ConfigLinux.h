#pragma once


#include "../../../../header.h"
#include "../../serialize/Serializer.h"

namespace zzz::engine
{
	class ConfigLinux final : public ISerializable
	{
	public:
		ConfigLinux();
		~ConfigLinux() = default;

	private:
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const Serializer& s) const override;
		[[nodiscard]] std::expected<void, std::string> DeSerialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& s) override;
	};
}

