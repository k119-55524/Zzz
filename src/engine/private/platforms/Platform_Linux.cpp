#include <foundation.h>

#include "../core/config/platforms/ConfigLinux.h"
#include "input/platforms/InputLinux.h"
#include "specific/Wayland/xdg-shell-client-protocol.h"

using namespace zzz::engine;

namespace
{
	struct RegistryData
	{
		wl_compositor**	compositor;
		wl_shm**		shm;
		xdg_wm_base**	xdgWmBase;
		wl_output**		output;
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
			// version 3+ required for wl_surface_set_buffer_scale
		*rd->compositor = static_cast<wl_compositor*>(wl_registry_bind(registry, name, &wl_compositor_interface, 3));
		}
		else if (std::string_view(interface) == wl_shm_interface.name)
		{
			*rd->shm = static_cast<wl_shm*>(wl_registry_bind(registry, name, &wl_shm_interface, 1));
		}
		else if (std::string_view(interface) == xdg_wm_base_interface.name)
		{
			*rd->xdgWmBase = static_cast<xdg_wm_base*>(wl_registry_bind(registry, name, &xdg_wm_base_interface, 1));
		}
		else if (std::string_view(interface) == wl_output_interface.name && !*rd->output)
		{
			// version 2 required for the scale event
			*rd->output = static_cast<wl_output*>(wl_registry_bind(registry, name, &wl_output_interface, 2));
		}
	}

	void OnRegistryGlobalRemove(void* /*data*/, wl_registry* /*registry*/, uint32_t /*name*/)
	{
	}

	const wl_registry_listener g_RegistryListener =
	{
		.global			= OnRegistryGlobal,
		.global_remove	= OnRegistryGlobalRemove,
	};

	void OnOutputGeometry(void*, wl_output*, int32_t, int32_t, int32_t, int32_t, int32_t, const char*, const char*, int32_t) {}
	void OnOutputMode(void*, wl_output*, uint32_t, int32_t, int32_t, int32_t) {}
	void OnOutputDone(void*, wl_output*) {}

	void OnOutputScale(void* data, wl_output*, int32_t factor)
	{
		*static_cast<int32_t*>(data) = factor;
		DOut("wl_output scale = {}", factor);
	}

	const wl_output_listener g_OutputListener =
	{
		.geometry = OnOutputGeometry,
		.mode     = OnOutputMode,
		.done     = OnOutputDone,
		.scale    = OnOutputScale,
	};

	void OnPing(
		void*,
		xdg_wm_base* wmBase,
		uint32_t serial)
	{
		DOut("OnPing");

		xdg_wm_base_pong(wmBase, serial);
	}

	const xdg_wm_base_listener g_WmBaseListener =
	{
		.ping = OnPing
	};
}

// These pointers were members of PlatformLinux, but now they can be part of NativeAppData
// or we can just keep them as static globals if there is only one Platform instance.
// Wait, PlatformLinux had them as members. Let's add them to NativeAppData in Linux.h
// For now, let's keep them as static globals in this file to avoid changing headers outside.
static wl_display* m_Display = nullptr;
static wl_registry* m_Registry = nullptr;
static wl_compositor* m_Compositor = nullptr;
static wl_shm* m_Shm = nullptr;
static xdg_wm_base* m_XdgWmBase = nullptr;
static wl_output* m_Output = nullptr;
static int32_t m_ScaleFactor = 1;

void Platform::InitializePlatformSpecific()
{
	try
	{
		m_Display = wl_display_connect(nullptr);
		if (!m_Display)
			THROW_RUNTIME("wl_display_connect() failed.");

		m_Registry = wl_display_get_registry(m_Display);
		if (!m_Registry)
			THROW_RUNTIME("wl_display_get_registry() failed.");

		RegistryData rd{ &m_Compositor, &m_Shm, &m_XdgWmBase, &m_Output };
		if (wl_registry_add_listener(m_Registry, &g_RegistryListener, &rd) != 0)
			THROW_RUNTIME("wl_registry_add_listener() failed.");

		// First roundtrip: receive registry globals
		if (wl_display_roundtrip(m_Display) == -1)
			THROW_RUNTIME("wl_display_roundtrip() failed.");

		if (!m_Compositor)
			THROW_RUNTIME("wl_compositor not found.");

		if (!m_Shm)
			THROW_RUNTIME("wl_shm not found.");

		if (!m_XdgWmBase)
			THROW_RUNTIME("xdg_wm_base not found.");

		xdg_wm_base_add_listener(m_XdgWmBase, &g_WmBaseListener, nullptr);

		// Second roundtrip: receive wl_output properties (scale, geometry, etc.)
		if (m_Output)
			wl_output_add_listener(m_Output, &g_OutputListener, &m_ScaleFactor);

		wl_display_roundtrip(m_Display);
	}
	catch (const std::exception& e)
	{
		ShutdownPlatformSpecific();
		THROW_RUNTIME("Exception initialize: {}.", e.what());
	}
	catch (...)
	{
		ShutdownPlatformSpecific();
		THROW_RUNTIME("Unknown exception occurred.");
	}

	DOut("PlatformLinux initialized: OK.");
}

void Platform::ShutdownPlatformSpecific()
{
	if (m_Output)
	{
		wl_output_destroy(m_Output);
		m_Output = nullptr;
	}

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

	if (m_Shm)
	{
		wl_shm_destroy(m_Shm);
		m_Shm = nullptr;
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
