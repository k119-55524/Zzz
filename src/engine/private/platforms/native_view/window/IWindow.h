#pragma once

#include "../../platforms/IPlatform.h"
#include "../../../core/templates/Event.h"

namespace zzz::engine
{
	class IInput;

	class IWindow
	{
	public:
		IWindow() = delete;
		IWindow(const std::shared_ptr<IPlatform> platform, const std::shared_ptr<IInput> input);
		virtual ~IWindow() = default;

		[[nodiscard]] virtual std::expected<void, std::string> Initialize(const std::string_view appName) = 0;

		Event<void> onCloseRequested;

	protected:
		const std::shared_ptr<IPlatform> m_Platform;
		const std::shared_ptr<IInput>    m_Input;
		Size2D<> m_WinSize;
	};
}
