#if defined(Z_LINUX)

#include "WinLinux.h"
#include "../../platforms/PlatformLinux.h"
#include "../../platforms/lLinux/Wayland/xdg-shell-client-protocol.h"

using namespace zzz::engine;

namespace
{
	void OnXdgSurfaceConfigure(
		void* data,
		xdg_surface* surface,
		uint32_t serial)
	{
		DOut("OnXdgSurfaceConfigure serial={}", serial);

		xdg_surface_ack_configure(surface, serial);

		auto* self = static_cast<WinLinux*>(data);

		wl_surface_commit(self->GetSurface());
	}

	const xdg_surface_listener g_XdgSurfaceListener =
	{
		.configure = OnXdgSurfaceConfigure
	};
}

WinLinux::WinLinux(const std::shared_ptr<IPlatform> platform) :
	IWindow(platform),
	m_Surface{nullptr},
	m_XdgSurface{nullptr},
	m_XdgToplevel{nullptr}
{
}

WinLinux::~WinLinux()
{
	Shutdown();
}

void WinLinux::Shutdown()
{
	if (m_XdgToplevel)
	{
		xdg_toplevel_destroy(m_XdgToplevel);
		m_XdgToplevel = nullptr;
	}

	if (m_XdgSurface)
	{
		xdg_surface_destroy(m_XdgSurface);
		m_XdgSurface = nullptr;
	}

	if (m_Surface)
	{
		wl_surface_destroy(m_Surface);
		m_Surface = nullptr;
	}
}

std::expected<void, std::string> WinLinux::Initialize(const std::string_view appName)
{
	try
	{
		auto platform = std::dynamic_pointer_cast<PlatformLinux>(m_Platform);
		ensure(platform != nullptr, "Platform is not PlatformLinux.");

		m_Surface = wl_compositor_create_surface(platform->GetCompositor());
		if (!m_Surface)
			return UNEXPECTED("wl_compositor_create_surface() failed.");

		m_XdgSurface = xdg_wm_base_get_xdg_surface(platform->GetXdgWmBase(), m_Surface);
		if (!m_XdgSurface)
			return UNEXPECTED("xdg_wm_base_get_xdg_surface() failed.");

		if (xdg_surface_add_listener(
			m_XdgSurface,
			&g_XdgSurfaceListener,
			this) != 0)
		{
			return UNEXPECTED("xdg_surface_add_listener() failed.");
		}

		m_XdgToplevel = xdg_surface_get_toplevel(m_XdgSurface);
		if (!m_XdgToplevel)
			return UNEXPECTED("xdg_surface_get_toplevel() failed.");

		xdg_toplevel_set_title(m_XdgToplevel, appName.data());
		wl_surface_commit(m_Surface);
		// wl_display_roundtrip(platform->GetDisplay());

		wl_display_roundtrip(platform->GetDisplay());
		wl_display_roundtrip(platform->GetDisplay());

		int err = wl_display_get_error(platform->GetDisplay());
		DOut("Wayland error = {}", err);
	}
	catch (const std::exception& e)
	{
		Shutdown();
		return UNEXPECTED("Exception initialize: {}.", e.what());
	}
	catch (...)
	{
		Shutdown();
		return UNEXPECTED("Unknown exception occurred.");
	}

	DOut("WinLinux initialized: OK.");

	return {};
}
#endif // defined(Z_LINUX)
