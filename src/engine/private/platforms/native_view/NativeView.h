#pragma once

#include "../../../header.h"

#include "../platforms/IPlatform.h"
#include "../../factories/PlatformFactory.h"
#include "../../platforms/native_view/window/IWindow.h"

namespace zzz::engine
{
	class NativeView final
	{
		Z_NO_MOVE(NativeView);

	public:
		NativeView() = delete;
		NativeView(std::shared_ptr<IPlatform> platform);
		~NativeView() = default;

		inline std::shared_ptr<IWindow> GetWindow() const noexcept { return m_Window; }

	private:
		void Initialize();

		std::shared_ptr<IPlatform> m_Platform;
		PlatformFactory m_Factory;
		std::shared_ptr<IWindow> m_Window;
	};
}
