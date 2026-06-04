#pragma once

#include "../../../core/config/EngineConfig.h"

namespace zzz::engine
{
	class IWindow
	{
	public:
		IWindow() = delete;
		IWindow(const EngineConfig& config);
		virtual ~IWindow() = default;

		[[nodiscard]] virtual std::expected<void, std::string> Initialize(const std::string_view appName) = 0;

	protected:
		const EngineConfig& m_Config;
	};
}