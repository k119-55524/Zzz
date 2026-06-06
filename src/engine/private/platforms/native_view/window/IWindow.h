#pragma once

#include "../../platforms/IPlatform.h"
#include "../../../core/config/EngineConfig.h"

namespace zzz::engine
{
	class IWindow
	{
	public:
		IWindow() = delete;
		IWindow(const std::shared_ptr<IPlatform> platform);
		virtual ~IWindow() = default;

		[[nodiscard]] virtual std::expected<void, std::string> Initialize(const std::string_view appName) = 0;

	protected:
		const std::shared_ptr<IPlatform> m_Platform;
	};
}