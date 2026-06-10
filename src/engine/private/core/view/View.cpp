#include "View.h"
#include "../../platforms/input/Input.h"
#include "../../platforms/window/Window.h"

using namespace zzz::engine;

View::View(std::shared_ptr<Platform> platform) :
	m_Platform{ platform }
{
	ensure(m_Platform != nullptr, "Platform must not be null.");

	Initialize();
}

void View::Initialize()
{
	m_Input = zzz::safe_make_shared<Input>();
	auto inputRes = m_Input->Initialize();
	if (!inputRes)
		THROW_RUNTIME("Failed to initialize input system: {}.", inputRes.error());

	m_Window = zzz::safe_make_shared<Window>(m_Platform, m_Input);
	auto res = m_Window->Initialize(m_Platform->GetAppName());
	if (!res)
		THROW_RUNTIME("Failed to initialize window: {}.", res.error());
}
