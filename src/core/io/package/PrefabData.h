#pragma once

#include <string>
#include <string_view>
#include "core/Serialize/Serializer.h"

namespace zzz::core
{
	class PrefabData final : public ISerializable
	{
	public:
		PrefabData() = default;

		inline void LogFileBlock(std::string_view indentation = {}) const
		{
			DOut(Assets, "{}[PrefabData]", indentation);
		}

	protected:
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
