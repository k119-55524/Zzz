#pragma once

#include "../../../core/Config/ConfigManager.h"

namespace zzz::engine
{
	class IWindow abstract
	{
	public:
		IWindow() = delete;
		IWindow(const PlatformConfig& platformConfig);
		virtual ~IWindow() = default;

		[[nodiscard]] virtual std::expected<void, std::string> Initialize() = 0;

	protected:
		const PlatformConfig& m_PlatformConfig;
	};
}