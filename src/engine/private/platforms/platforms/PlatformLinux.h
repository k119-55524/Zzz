#pragma once

#if defined(Z_LINUX)

#include "IPlatform.h"

struct wl_display;
struct wl_registry;
struct wl_compositor;
struct xdg_wm_base;

namespace zzz::engine
{
	class PlatformLinux final : public IPlatform
	{
	public:
		PlatformLinux(std::string_view appName, std::shared_ptr<void> platformData = nullptr);
		~PlatformLinux() override;

	private:
		void InitializeImpl() override;
		void InitializeWayland();
		void ShutdownWayland();

		wl_display* m_Display;
		wl_registry* m_Registry;
		wl_compositor* m_Compositor;
		xdg_wm_base* m_XdgWmBase;
	};
}
#endif // defined(Z_LINUX)