#pragma once

#include "Window_Common.h"
#include "../input/Input.h"

#if Z_WINDOWS
#include <windows.h>
#endif

namespace zzz::engine
{
	class InputEditor;

	class WinEditor final : public WindowBase
	{
	public:
		WinEditor() = delete;
		WinEditor(const Platform& platform, const std::shared_ptr<Input> input, WindowCallbacks callbacks);
		~WinEditor();

		[[nodiscard]] std::expected<void, std::string> Initialize(const std::string_view appName);

	private:
		HWND m_hWnd;
	};
}
