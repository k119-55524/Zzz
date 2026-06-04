#pragma once

#include "../../../header.h"

#include "../../core/config/EngineConfig.h"
#include "../../factories/PlatformFactory.h"
#include "../../platforms/native_view/window/IWindow.h"

namespace zzz::engine
{
	class NativeView final
	{
		Z_NO_MOVE(NativeView);

	public:
		NativeView() = delete;
		NativeView(const std::string_view appName, const EngineConfig& config);
		~NativeView() = default;

	private:
		void Initialize(const std::string_view appName);

		const EngineConfig& m_Config;
		PlatformFactory m_Factory;
		std::shared_ptr<IWindow> m_Window;
	};
}
