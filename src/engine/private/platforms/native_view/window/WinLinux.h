#pragma once

#if defined(Z_LINUX)

#include "IWindow.h"

struct wl_surface;
struct wl_buffer;
struct xdg_surface;
struct xdg_toplevel;

#include "../../../../header.h"

namespace zzz::engine
{
	class WinLinux final : public IWindow
	{
	public:
		WinLinux() = delete;
		WinLinux(const std::shared_ptr<IPlatform> platform, const std::shared_ptr<IInput> input);
		~WinLinux() override;

		[[nodiscard]] virtual std::expected<void, std::string> Initialize(const std::string_view appName) override;
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
#endif // defined(Z_LINUX)