#pragma once

#include "../../../header.h"

#include "../../factories/PlatformFactory.h"
#include "../../core/Config/ConfigManager.h"
#include "../../platforms/native_view/window/IWindow.h"

namespace zzz::engine
{
	class NativeView final
	{
		Z_NO_MOVE(NativeView);

	public:
		NativeView(std::shared_ptr<ConfigManager> configManager);
		~NativeView();

	private:
		void Initialize();

		std::shared_ptr<ConfigManager> m_ConfigManager;
		PlatformFactory m_PlatformFactory;
		std::shared_ptr<IWindow> m_Window;

	};
}