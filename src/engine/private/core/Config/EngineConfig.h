#pragma once

#include <foundation.h>

#include "../utils/version.h"
#include "../templates/Size2D.h"
#include "../serialize/Serializer.h"

#include "platforms/ConfigMSWin.h"
#include "platforms/ConfigLinux.h"

namespace zzz::engine
{
#if defined(Z_WINDOWS)
	typedef zzz::engine::ConfigMSWin PlatformConfig;
#elif defined(Z_LINUX)
	typedef zzz::engine::ConfigLinux PlatformConfig;
#else
#error >>>>> Unsupported platform. No window implementation available.
#endif

	class EngineConfig final : public ISerializable
	{
	public:
		EngineConfig();
		~EngineConfig() = default;

		inline const PlatformConfig& GetPlatformConfig() const noexcept { return m_PlatformConfig; }

	private:
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const Serializer& s) const override;
		[[nodiscard]] std::expected<void, std::string> DeSerialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& s) override;

		Version m_Version;
		PlatformConfig m_PlatformConfig;
	};
}