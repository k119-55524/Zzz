#pragma once

#include <string_view>
#include <logger/logger.h>
#include "core/Serialize/Serializer.h"

namespace zzz::core
{
	class ProjectPlatformDataLinux final : public ISerializable
	{
	public:
		ProjectPlatformDataLinux() = default;

		inline void LogFileBlock(std::string_view indentation = {}) const
		{
			DOut("{}[ProjectPlatformDataLinux]", indentation);
		}

	private:
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>&, const Serializer&) const override
		{
			return {};
		}

		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte>, std::size_t&, const Serializer&) override
		{
			return {};
		}
	};
}
