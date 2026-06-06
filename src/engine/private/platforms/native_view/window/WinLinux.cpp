#if defined(Z_LINUX)

#include "WinLinux.h"
#include "../../platforms/PlatformLinux.h"
#include "../../platforms/lLinux/Wayland/xdg-shell-client-protocol.h"

#include <sys/mman.h>
#include <unistd.h>

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

		if (self->GetBuffer())
		{
			wl_surface_attach(self->GetSurface(), self->GetBuffer(), 0, 0);
			wl_surface_damage(self->GetSurface(), 0, 0, 0x7FFFFFFF, 0x7FFFFFFF);
		}

		wl_surface_commit(self->GetSurface());
	}

	const xdg_surface_listener g_XdgSurfaceListener =
	{
		.configure = OnXdgSurfaceConfigure
	};

	void OnToplevelConfigure(void*, xdg_toplevel*, int32_t /*w*/, int32_t /*h*/, wl_array*)
	{}

	void OnToplevelClose(void* data, xdg_toplevel*)
	{
		static_cast<WinLinux*>(data)->onCloseRequested();
	}

	const xdg_toplevel_listener g_ToplevelListener =
	{
		.configure = OnToplevelConfigure,
		.close     = OnToplevelClose,
	};
}

WinLinux::WinLinux(const std::shared_ptr<IPlatform> platform) :
	IWindow(platform),
	m_Surface{nullptr},
	m_Buffer{nullptr},
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

	if (m_Buffer)
	{
		wl_buffer_destroy(m_Buffer);
		m_Buffer = nullptr;
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
		xdg_toplevel_add_listener(m_XdgToplevel, &g_ToplevelListener, this);

		{
			constexpr int W = 800, H = 600, stride = W * 4;
			constexpr int size = stride * H;

			int fd = memfd_create("zzz_shm", MFD_CLOEXEC);
			if (fd >= 0)
			{
				ftruncate(fd, size);
				void* px = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
				if (px != MAP_FAILED)
				{
					std::fill_n(static_cast<uint32_t*>(px), W * H, uint32_t{0xFF1E1E2E});
					munmap(px, size);
				}
				wl_shm_pool* pool = wl_shm_create_pool(platform->GetShm(), fd, size);
				close(fd);
				m_Buffer = wl_shm_pool_create_buffer(pool, 0, W, H, stride, WL_SHM_FORMAT_ARGB8888);
				wl_shm_pool_destroy(pool);
			}
		}

		wl_surface_commit(m_Surface);

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
