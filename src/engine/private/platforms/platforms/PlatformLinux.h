#pragma once

#if defined(Z_LINUX)

#include "IPlatform.h"

struct wl_display;
struct wl_registry;
struct wl_compositor;
struct wl_shm;
struct wl_output;
struct xdg_wm_base;

namespace zzz::engine
{
	class PlatformLinux final : public IPlatform
	{
	public:
		PlatformLinux(std::string_view appName, std::shared_ptr<PlatformNativeData> platformData = nullptr);
		~PlatformLinux() override;

		inline wl_compositor* GetCompositor()  const noexcept { return m_Compositor; };
		inline wl_shm*        GetShm()         const noexcept { return m_Shm; };
		inline xdg_wm_base*   GetXdgWmBase()   const noexcept { return m_XdgWmBase; };
		inline wl_display*    GetDisplay()     const noexcept { return m_Display; };
		inline int32_t        GetScaleFactor() const noexcept { return m_ScaleFactor; };

	private:
		void InitializeImpl() override;
		void InitializeWayland();
		void Shutdown();

		wl_display*		m_Display;
		wl_registry*	m_Registry;
		wl_compositor*	m_Compositor;
		wl_shm*			m_Shm;
		xdg_wm_base*	m_XdgWmBase;
		wl_output*		m_Output;
		int32_t			m_ScaleFactor;
	};
}
#endif // defined(Z_LINUX)