#pragma once

#include "../Platform.h"
#include "../../core/templates/Event.h"
#include "../../core/templates/Size2D.h"

#include "../input/Input.h"

namespace zzz::engine
{
	class WindowBase
	{
	public:
		WindowBase() = delete;
		WindowBase(const std::shared_ptr<Platform> platform, const std::shared_ptr<Input> input, std::function<void()> onWindowClose);
		virtual ~WindowBase() = default;

	protected:
		const std::shared_ptr<Platform> m_Platform;
		const std::shared_ptr<Input> m_Input;
		Size2D<> m_WinSize;

		std::function<void()> OnWindowClose;
	};
}
