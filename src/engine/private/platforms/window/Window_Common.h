#pragma once

#include "../Platform.h"
#include "../input/Input.h"
#include "../../core/templates/Size2D.h"


namespace zzz::engine
{
	enum class eWinResize : zU32
	{
		Show,
		Hide,
		Resize
	};

	class WindowBase
	{
	public:
		WindowBase() = delete;
		WindowBase(const std::shared_ptr<Platform> platform, const std::shared_ptr<Input> input, std::function<void()> onWindowClose);
		virtual ~WindowBase() = default;

		std::function<void()> OnClose;
		std::function<void(Size2D<>, eWinResize)> OnResize;

	protected:
		const std::shared_ptr<Platform> m_Platform;
		const std::shared_ptr<Input> m_Input;
		Size2D<> m_WinSize;
	};
}
