#pragma once

#include <foundation.h>

#include "../utils/version.h"
#include "../templates/Size2D.h"
#include "../serialize/Serializer.h"

#include "platforms/IConfig.h"

namespace zzz::engine
{
	class EngineConfig final : public ISerializable
	{
	public:
		EngineConfig() = delete;
		explicit EngineConfig(std::shared_ptr<IConfig> platformConfig);
		~EngineConfig() = default;

		inline const IConfig& GetPlatformConfig() const noexcept { return *m_PlatformConfig; }

	private:
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const Serializer& s) const override;
		[[nodiscard]] std::expected<void, std::string> DeSerialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& s) override;

		Version m_Version;
		std::shared_ptr<IConfig> m_PlatformConfig;
	};
}