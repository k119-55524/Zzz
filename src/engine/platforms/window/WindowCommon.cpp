#include "WindowCommon.h"
#include "../Platform.h"
#include "../monitor/IMonitorProvider.h"

using namespace zzz::engine;

WindowBase::WindowBase(
	const Platform& platform,
	const std::shared_ptr<Input> input,
	WindowCallbacks callbacks) :
	m_Platform{ platform },
	m_Input{ input },
	m_Callbacks{ std::move(callbacks) }
{
	ensure(m_Callbacks.OnClose != nullptr, "OnClose не должен быть null.");
	ensure(m_Callbacks.OnSurfaceCreated != nullptr, "OnSurfaceCreated не должен быть null.");
}

zzz::math::Size2D<zzz::core::zU32> WindowBase::GetPhysicalClientSize() const noexcept
{
	const auto clientSize = GetClientRect().size;
	const auto& monitorProvider = m_Platform.GetMonitorProvider();
	const auto monitor = monitorProvider.GetMonitorForRect(GetClientRect());
	const float scale = monitor.GetScaleFactor();

	return zzz::math::Size2D<zzz::core::zU32>{
		static_cast<zzz::core::zU32>(static_cast<float>(clientSize.width) * scale),
		static_cast<zzz::core::zU32>(static_cast<float>(clientSize.height) * scale)
	};
}
