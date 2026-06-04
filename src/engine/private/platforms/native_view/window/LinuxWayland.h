#pragma once

#if defined(Z_LINUX)

#include <wayland-client.h>
#include "xdg-shell-client-protocol.h"

#include "IWindow.h"
#include "../../../../header.h"
#include "../../../core/config/EngineConfig.h"

#include "IWindow.h"

namespace zzz::engine
{
	class LinuxWayland final : public IWindow
	{
	public:
		LinuxWayland() = delete;
		LinuxWayland(const EngineConfig& config);
		~LinuxWayland() = default;

		[[nodiscard]] virtual std::expected<void, std::string> Initialize(const std::string_view appName) override;

	private:
		wl_display* m_Display{};
		wl_registry* m_Registry{};
		wl_compositor* m_Compositor{};

		wl_surface* m_Surface{};

		xdg_wm_base* m_XdgWmBase{};
		xdg_surface* m_XdgSurface{};
		xdg_toplevel* m_XdgToplevel{};
	};
}
#endif // defined(Z_LINUX)