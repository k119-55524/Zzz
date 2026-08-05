#pragma once


#include "WindowCommon.h"

struct wl_surface;
struct wl_buffer;
struct xdg_surface;
struct xdg_toplevel;

#include "../../header.h"

namespace zzz::engine
{
	class WinLinux final : public WindowBase
	{
	public:
		WinLinux() = delete;
		WinLinux(const Platform& platform, const std::shared_ptr<Input> input, WindowCallbacks callbacks);
		~WinLinux();

		[[nodiscard]] std::expected<void, std::string> Initialize(const std::string_view appName);
		inline wl_surface* GetSurface() const noexcept { return m_Surface; };
		inline wl_buffer*  GetBuffer()  const noexcept { return m_Buffer; };

	private:
		void Shutdown();

		wl_surface*  m_Surface;
		wl_buffer*   m_Buffer;
		xdg_surface* m_XdgSurface;
		xdg_toplevel* m_XdgToplevel;
	};
}
