#pragma once

#if defined(Z_LINUX)

#include "IWindow.h"

struct wl_surface;
struct xdg_surface;
struct xdg_toplevel;

#include "../../../../header.h"

namespace zzz::engine
{
	class WinLinux final : public IWindow
	{
	public:
		WinLinux() = delete;
		WinLinux(const std::shared_ptr<IPlatform> platform);
		~WinLinux() override;

		[[nodiscard]] virtual std::expected<void, std::string> Initialize(const std::string_view appName) override;
		inline wl_surface* GetSurface() const noexcept { return m_Surface; };

	private:
		void Shutdown();

		wl_surface* m_Surface;
		xdg_surface* m_XdgSurface;
		xdg_toplevel* m_XdgToplevel;
	};
}
#endif // defined(Z_LINUX)