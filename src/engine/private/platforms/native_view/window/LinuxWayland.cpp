#if defined(Z_LINUX)

#include "xdg-shell-client-protocol.h"
#include "LinuxWayland.h"

using namespace zzz::engine;

namespace
{
	struct RegistryData
	{
		wl_compositor**	compositor;
		xdg_wm_base**	xdgWmBase;
	};

	void OnRegistryGlobal(
		void* data,
		wl_registry* registry,
		uint32_t name,
		const char* interface,
		uint32_t /*version*/)
	{
		auto* rd = static_cast<RegistryData*>(data);

		if (std::string_view(interface) == wl_compositor_interface.name)
		{
			*rd->compositor = static_cast<wl_compositor*>(wl_registry_bind(registry, name, &wl_compositor_interface, 1));
		}
		else if (std::string_view(interface) == xdg_wm_base_interface.name)
		{
			*rd->xdgWmBase = static_cast<xdg_wm_base*>(wl_registry_bind(registry, name, &xdg_wm_base_interface, 1));
		}
	}

	void OnRegistryGlobalRemove(void* /*data*/, wl_registry* /*registry*/, uint32_t /*name*/)
	{}

	const wl_registry_listener g_RegistryListener =
	{
		.global			= OnRegistryGlobal,
		.global_remove	= OnRegistryGlobalRemove,
	};
}

LinuxWayland::LinuxWayland(const EngineConfig& config) :
	IWindow(config)
{}

std::expected<void, std::string> LinuxWayland::Initialize(const std::string_view appName)
{
	m_Display = wl_display_connect(nullptr);
	if (!m_Display)
		return UNEXPECTED("wl_display_connect() failed.");

	m_Registry = wl_display_get_registry(m_Display);
	if (!m_Registry)
		return UNEXPECTED("wl_display_get_registry() failed.");

	RegistryData rd{ &m_Compositor, &m_XdgWmBase };
	wl_registry_add_listener(m_Registry, &g_RegistryListener, &rd);
	wl_display_roundtrip(m_Display);

	if (!m_Compositor)
		return UNEXPECTED("wl_compositor not found.");

	if (!m_XdgWmBase)
		return UNEXPECTED("xdg_wm_base not found.");

	m_Surface = wl_compositor_create_surface(m_Compositor);
	if (!m_Surface)
		return UNEXPECTED("wl_compositor_create_surface() failed.");

	m_XdgSurface = xdg_wm_base_get_xdg_surface(m_XdgWmBase, m_Surface);
	if (!m_XdgSurface)
		return UNEXPECTED("xdg_wm_base_get_xdg_surface() failed.");

	m_XdgToplevel = xdg_surface_get_toplevel(m_XdgSurface);
	if (!m_XdgToplevel)
		return UNEXPECTED("xdg_surface_get_toplevel() failed.");

	xdg_toplevel_set_title(m_XdgToplevel, appName.data());
	wl_surface_commit(m_Surface);
	wl_display_roundtrip(m_Display);

	return {};
}
#endif // defined(Z_LINUX)
