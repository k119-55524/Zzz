#pragma once

#include "../../Platform.h"
#include "../../../core/templates/Event.h"

namespace zzz::engine
{
	class IInput;

	class IWindow
	{
	public:
		IWindow() = delete;
		IWindow(const std::shared_ptr<Platform> platform, const std::shared_ptr<IInput> input);
		virtual ~IWindow() = default;

		[[nodiscard]] virtual std::expected<void, std::string> Initialize(const std::string_view appName) = 0;

		Event<void> onCloseRequested;

	protected:
		const std::shared_ptr<Platform> m_Platform;
		const std::shared_ptr<IInput>    m_Input;
		Size2D<> m_WinSize;
	};
}
