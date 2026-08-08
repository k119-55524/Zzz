#pragma once

#include <string_view>
#include <logger/logger.h>
#include "core/Serialize/Serializer.h"

namespace zzz::core
{
	class ProjectPlatformDataAndroid final : public ISerializable
	{
	public:
		ProjectPlatformDataAndroid() = default;

		inline void LogFileBlock(std::string_view indentation = {}) const
		{
			DOut("{}[ProjectPlatformDataAndroid]", indentation);
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
