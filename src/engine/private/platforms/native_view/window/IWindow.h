#pragma once

#include "../../../core/Config/ConfigManager.h"

namespace zzz::engine
{
	class IWindow abstract
	{
	public:
		IWindow() = delete;
		IWindow(std::shared_ptr<ConfigManager> configManager);
		virtual ~IWindow() = default;

		[[nodiscard]] virtual std::expected<void, std::string> Initialize() = 0;

	protected:
		std::shared_ptr<ConfigManager> m_ConfigManager;
	};
}