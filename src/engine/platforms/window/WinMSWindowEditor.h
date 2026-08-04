#pragma once

#include "Window_Common.h"
#include "../input/Input.h"

#if Z_WINDOWS
#include <windows.h>
#endif

namespace zzz::engine
{
	class InputEditor;

	class WinMSWindowEditor final : public WindowBase
	{
	public:
		WinMSWindowEditor() = delete;
		WinMSWindowEditor(const Platform& platform, const std::shared_ptr<Input> input, WindowCallbacks callbacks);
		~WinMSWindowEditor();

		[[nodiscard]] std::expected<void, std::string> Initialize(const std::string_view appName, void* data = nullptr);

		HWND GetHWnd() const noexcept { return m_hWnd; }

	private:
		HWND m_hWnd = nullptr;
	};
}