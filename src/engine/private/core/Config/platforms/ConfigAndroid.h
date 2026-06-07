#pragma once

#if defined(Z_ANDROID)

#include "../../../../header.h"
#include "IConfig.h"

namespace zzz::engine
{
	class ConfigAndroid final : public IConfig
	{
	public:
		ConfigAndroid();
		~ConfigAndroid() = default;

	private:
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const Serializer& s) const override;
		[[nodiscard]] std::expected<void, std::string> DeSerialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& s) override;
	};
}

#endif // Z_ANDROID
