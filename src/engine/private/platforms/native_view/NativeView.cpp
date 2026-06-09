#include "NativeView.h"
#include "../../factories/EngineFactory.h"

using namespace zzz::engine;

NativeView::NativeView(std::shared_ptr<Platform> platform) :
	m_Platform{ platform }
{
	ensure(m_Platform != nullptr, "Platform must not be null.");

	Initialize();
}

void NativeView::Initialize()
{
	m_Input = m_Platform->GetFactory()->CreateInput();
	auto inputRes = m_Input->Initialize();
	if (!inputRes)
		THROW_RUNTIME("Failed to initialize input system: {}.", inputRes.error());

	m_Window = m_Platform->GetFactory()->CreateAppWin(m_Platform, m_Input);
	auto res = m_Window->Initialize(m_Platform->GetAppName());
	if (!res)
		THROW_RUNTIME("Failed to initialize window: {}.", res.error());
}
