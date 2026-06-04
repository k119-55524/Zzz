#pragma once

#include "../Utils/version.h"
//#include "../templates/Size2D.h"
#include "../Serialize/Serializer.h"

#include "platforms/ConfigMSWin.h"

namespace zzz::engine
{
#if defined(Z_WINDOWS)
	typedef zzz::engine::ConfigMSWin PlatformConfig;
#else
#error >>>>> Unsupported platform. No window implementation available.
#endif

	class EngineConfig final : public ISerializable
	{
	public:
		EngineConfig();

		//inline const Size2D<LONG>& GetWinSize() const noexcept { return m_WinSize; }
		inline const PlatformConfig& GetPlatformConfig() const noexcept { return m_PlatformConfig; }

	private:
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const Serializer& s) const override;
		[[nodiscard]] std::expected<void, std::string> DeSerialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& s) override;

		Version m_Version;
		//Size2D<LONG> m_WinSize;
		PlatformConfig m_PlatformConfig;
	};
}