#if defined(Z_LINUX)

#include "WinLinux.h"
#include "../../platforms/lLinux/Wayland/xdg-shell-client-protocol.h"

using namespace zzz::engine;

WinLinux::WinLinux(const std::shared_ptr<IPlatform> platform) :
	IWindow(platform)
{}

std::expected<void, std::string> WinLinux::Initialize(const std::string_view appName)
{
	//m_Surface = wl_compositor_create_surface(m_Compositor);
	//if (!m_Surface)
	//	return UNEXPECTED("wl_compositor_create_surface() failed.");

	//m_XdgSurface = xdg_wm_base_get_xdg_surface(m_XdgWmBase, m_Surface);
	//if (!m_XdgSurface)
	//	return UNEXPECTED("xdg_wm_base_get_xdg_surface() failed.");

	//m_XdgToplevel = xdg_surface_get_toplevel(m_XdgSurface);
	//if (!m_XdgToplevel)
	//	return UNEXPECTED("xdg_surface_get_toplevel() failed.");

	//xdg_toplevel_set_title(m_XdgToplevel, appName.data());
	//wl_surface_commit(m_Surface);
	//wl_display_roundtrip(m_Display);

	return {};
}
#endif // defined(Z_LINUX)
