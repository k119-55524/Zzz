#if defined(Z_LINUX)

#include "PlatformLinux.h"
#include "lLinux/Wayland/xdg-shell-client-protocol.h"

using namespace zzz::engine;

namespace
{
	struct RegistryData
	{
		wl_compositor** compositor;
		xdg_wm_base** xdgWmBase;
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
		.global = OnRegistryGlobal,
		.global_remove = OnRegistryGlobalRemove,
	};
}

PlatformLinux::PlatformLinux(std::string_view appName, std::shared_ptr<void> platformData) :
	IPlatform(appName, platformData),
	m_Display{ nullptr },
	m_Registry{ nullptr },
	m_Compositor{ nullptr },
	m_XdgWmBase{ nullptr }
{}

PlatformLinux::~PlatformLinux()
{
	ShutdownWayland();
}

void PlatformLinux::InitializeImpl()
{
	InitializeWayland();
}

void PlatformLinux::InitializeWayland()
{
	m_Display = wl_display_connect(nullptr);
	if (!m_Display)
		THROW_RUNTIME("wl_display_connect() failed.");

	m_Registry = wl_display_get_registry(m_Display);
	if (!m_Registry)
		THROW_RUNTIME("wl_display_get_registry() failed.");

	RegistryData rd{ &m_Compositor, &m_XdgWmBase };
	wl_registry_add_listener(m_Registry, &g_RegistryListener, &rd);
	wl_display_roundtrip(m_Display);

	if (!m_Compositor)
		THROW_RUNTIME("wl_compositor not found.");

	if (!m_XdgWmBase)
		THROW_RUNTIME("xdg_wm_base not found.");

	DOut("PlatformLinux initialized: OK.");
}

void PlatformLinux::ShutdownWayland()
{
	if (m_XdgWmBase)
	{
		xdg_wm_base_destroy(m_XdgWmBase);
		m_XdgWmBase = nullptr;
	}

	if (m_Compositor)
	{
		wl_compositor_destroy(m_Compositor);
		m_Compositor = nullptr;
	}

	if (m_Registry)
	{
		wl_registry_destroy(m_Registry);
		m_Registry = nullptr;
	}

	if (m_Display)
	{
		wl_display_disconnect(m_Display);
		m_Display = nullptr;
	}
}
#endif // defined(Z_LINUX)