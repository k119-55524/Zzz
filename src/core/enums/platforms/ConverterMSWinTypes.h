#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <core/enums/platforms/eMSWinEnums.h>

namespace zzz::core
{

	class ConverterMSWinTypes final
	{
	public:
		ConverterMSWinTypes() = delete;

		// eMSWinWindowStyle
		[[nodiscard]] static constexpr DWORD ToNative(eMSWinWindowStyle style) noexcept
		{
			switch (style)
			{
			case eMSWinWindowStyle::OverlappedWindow: return WS_OVERLAPPEDWINDOW;
			case eMSWinWindowStyle::PopUp:            return WS_POPUP | WS_VISIBLE;
			case eMSWinWindowStyle::ToolWindow:       return WS_EX_TOOLWINDOW | WS_CAPTION | WS_SYSMENU;
			}
			return WS_OVERLAPPEDWINDOW;
		}

		[[nodiscard]] static constexpr eMSWinWindowStyle ToEngine(DWORD nativeStyle) noexcept
		{
			if (nativeStyle & WS_POPUP)      return eMSWinWindowStyle::PopUp;
			if (nativeStyle & WS_EX_TOOLWINDOW) return eMSWinWindowStyle::ToolWindow;
			return eMSWinWindowStyle::OverlappedWindow;
		}

		// eMSWinWindowMode
		[[nodiscard]] static constexpr DWORD ToNative(eMSWinWindowMode mode) noexcept
		{
			switch (mode)
			{
			case eMSWinWindowMode::Windowed:             return WS_OVERLAPPEDWINDOW;
			case eMSWinWindowMode::BorderlessFullscreen: return WS_POPUP;
			case eMSWinWindowMode::ExclusiveFullscreen:  return WS_POPUP;
			}
			return WS_OVERLAPPEDWINDOW;
		}
	};
}
