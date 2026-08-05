#pragma once

#include <string_view>
#include <logger/logger.h>
#include <core/Serialize/Serializer.h>

namespace zzz::core
{
	class ConfigiOS final : public ISerializable
	{
	public:
		ConfigiOS();
		~ConfigiOS() override = default;

		inline void LogFileBlock(std::string_view indentation = {}) const
		{
			DOut("{}[ConfigiOS]", indentation);
		}

	private:
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const Serializer& s) const override;
		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& s) override;
	};
}



