#pragma once

#if defined(Z_LINUX)

#include "IWindow.h"

struct wl_surface;
struct xdg_surface;
struct xdg_toplevel;

#include "../../../../header.h"
#include "../../../core/config/EngineConfig.h"

namespace zzz::engine
{
	class WinLinux final : public IWindow
	{
	public:
		WinLinux() = delete;
		WinLinux(const std::shared_ptr<IPlatform> platform);
		~WinLinux() = default;

		[[nodiscard]] virtual std::expected<void, std::string> Initialize(const std::string_view appName) override;

	private:
		wl_surface* m_Surface{};
		xdg_surface* m_XdgSurface{};
		xdg_toplevel* m_XdgToplevel{};
	};
}
#endif // defined(Z_LINUX)