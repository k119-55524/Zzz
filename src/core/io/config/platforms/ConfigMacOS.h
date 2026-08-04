#pragma once

#include <string_view>
#include <logger/logger.h>
#include <core/Serialize/Serializer.h>

namespace zzz::core
{
	using namespace zzz::core;
	class ConfigMacOS final : public ISerializable
	{
	public:
		ConfigMacOS();
		~ConfigMacOS() override = default;

		inline void LogFileBlock(std::string_view indentation = {}) const
		{
			DOut("{}[ConfigMacOS]", indentation);
		}

	private:
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const Serializer& s) const override;
		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& s) override;
	};
}



