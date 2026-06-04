#if defined(Z_LINUX)

#include "LinuxWayland.h"

using namespace zzz::engine;

LinuxWayland::LinuxWayland(const EngineConfig& config) :
	IWindow(config)
{
}

std::expected<void, std::string> LinuxWayland::Initialize(const std::string_view appName)
{
	m_Display = wl_display_connect(nullptr);

	if (!m_Display)
		return UNEXPECTED("wl_display_connect() failed.");

	m_Registry = wl_display_get_registry(m_Display);

	if (!m_Registry)
		return UNEXPECTED("wl_display_get_registry() failed.");

	wl_registry_add_listener(
		m_Registry,
		&g_RegistryListener,
		this);

	wl_display_roundtrip(m_Display);

	if (!m_Compositor)
		return UNEXPECTED("wl_compositor not found.");

	if (!m_XdgWmBase)
		return UNEXPECTED("xdg_wm_base not found.");

	m_Surface =
		wl_compositor_create_surface(
			m_Compositor);

	if (!m_Surface)
		return UNEXPECTED("wl_compositor_create_surface() failed.");

	m_XdgSurface =
		xdg_wm_base_get_xdg_surface(
			m_XdgWmBase,
			m_Surface);

	if (!m_XdgSurface)
		return UNEXPECTED("xdg_wm_base_get_xdg_surface() failed.");

	m_XdgToplevel =
		xdg_surface_get_toplevel(
			m_XdgSurface);

	if (!m_XdgToplevel)
		return UNEXPECTED("xdg_surface_get_toplevel() failed.");

	xdg_toplevel_set_title(
		m_XdgToplevel,
		appName.data());

	wl_surface_commit(m_Surface);

	wl_display_roundtrip(m_Display);

	return {};
}
#endif // defined(Z_LINUX)