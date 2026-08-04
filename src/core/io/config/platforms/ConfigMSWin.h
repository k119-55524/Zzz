#pragma once

#include <string_view>
#include <logger/logger.h>
#include <core/headers/MSWin.h>
#include <core/Serialize/Serializer.h>

namespace zzz::engine
{
	using namespace zzz::common;
	class ConfigMSWin final : public ISerializable
	{
	public:
		ConfigMSWin();
		~ConfigMSWin() = default;

		inline void LogFileBlock(std::string_view indentation = {}) const
		{
			DOut("{}[ConfigMSWin]", indentation);
		}

	private:
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const Serializer& s) const override;
		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& s) override;
	};
}




