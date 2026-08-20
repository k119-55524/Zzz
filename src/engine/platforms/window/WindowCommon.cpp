#include "WindowCommon.h"

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
