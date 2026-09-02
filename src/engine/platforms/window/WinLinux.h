#pragma once

#include "core/utils/Defines.h"

#if defined(Z_LINUX)

#include "WindowCommon.h"

struct wl_surface;
struct wl_buffer;
struct xdg_surface;
struct xdg_toplevel;

#include "../../header.h"

namespace zzz::engine
{
	class View;

	class WinLinux final : public WindowBase
	{
	public:
		WinLinux() = delete;
		WinLinux(const Platform& platform, const std::shared_ptr<Input> input, WindowCallbacks callbacks);
		~WinLinux();

		[[nodiscard]] std::expected<void, std::string> Initialize(const ViewPlatformData& windowSettings, const View* parentView = nullptr) override;
		[[nodiscard]] bool IsMinimized() const noexcept override { return false; }
		[[nodiscard]] bool IsMaximized() const override { return false; }
		[[nodiscard]] Rect2D<zI32> GetClientRect() const override { return Rect2D<zI32>{ Point2D<zI32>{0, 0}, Size2D<zI32>{static_cast<zI32>(m_WinSize.width), static_cast<zI32>(m_WinSize.height)} }; }
		void OnMonitorResolutionChanged() override {}

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
