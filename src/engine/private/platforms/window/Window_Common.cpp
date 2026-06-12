#include "Window_Common.h"

using namespace zzz::engine;

WindowBase::WindowBase(
	const std::shared_ptr<Platform> platform,
	const std::shared_ptr<Input> input,
	WindowCallbacks callbacks) :
	m_Platform{ platform },
	m_Input{ input },
	m_Callbacks{ std::move(callbacks) },
	IsActivate{ false }
{
	ensure(m_Callbacks.OnClose != nullptr, "OnClose must not be null.");
	ensure(m_Callbacks.OnSurfaceCreated != nullptr, "OnSurfaceCreated must not be null.");
}
