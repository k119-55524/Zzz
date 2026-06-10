#include <foundation.h>
#include <memory>
#include "Platform.h"

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

void Platform::InitializePlatformSpecific()
{
	m_NativeData = std::make_shared<NativeAppData>();
	try
	{
		m_NativeData->display = wl_display_connect(nullptr);
		if (!m_NativeData->display)
			THROW_RUNTIME("wl_display_connect() failed.");

		m_NativeData->registry = wl_display_get_registry(m_NativeData->display);
		if (!m_NativeData->registry)
			THROW_RUNTIME("wl_display_get_registry() failed.");

		RegistryData rd{ &m_NativeData->compositor, &m_NativeData->shm, &m_NativeData->xdgWmBase, &m_NativeData->output };
		if (wl_registry_add_listener(m_NativeData->registry, &g_RegistryListener, &rd) != 0)
			THROW_RUNTIME("wl_registry_add_listener() failed.");

		// First roundtrip: receive registry globals
		if (wl_display_roundtrip(m_NativeData->display) == -1)
			THROW_RUNTIME("wl_display_roundtrip() failed.");

		if (!m_NativeData->compositor)
			THROW_RUNTIME("wl_compositor not found.");

		if (!m_NativeData->shm)
			THROW_RUNTIME("wl_shm not found.");

		if (!m_NativeData->xdgWmBase)
			THROW_RUNTIME("xdg_wm_base not found.");

		xdg_wm_base_add_listener(m_NativeData->xdgWmBase, &g_WmBaseListener, nullptr);

		// Second roundtrip: receive wl_output properties (scale, geometry, etc.)
		if (m_NativeData->output)
			wl_output_add_listener(m_NativeData->output, &g_OutputListener, &m_NativeData->scaleFactor);

		wl_display_roundtrip(m_NativeData->display);
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
	if (m_NativeData->output)
	{
		wl_output_destroy(m_NativeData->output);
		m_NativeData->output = nullptr;
	}

	if (m_NativeData->xdgWmBase)
	{
		xdg_wm_base_destroy(m_NativeData->xdgWmBase);
		m_NativeData->xdgWmBase = nullptr;
	}

	if (m_NativeData->compositor)
	{
		wl_compositor_destroy(m_NativeData->compositor);
		m_NativeData->compositor = nullptr;
	}

	if (m_NativeData->shm)
	{
		wl_shm_destroy(m_NativeData->shm);
		m_NativeData->shm = nullptr;
	}

	if (m_NativeData->registry)
	{
		wl_registry_destroy(m_NativeData->registry);
		m_NativeData->registry = nullptr;
	}

	if (m_NativeData->display)
	{
		wl_display_disconnect(m_NativeData->display);
		m_NativeData->display = nullptr;
	}
}
