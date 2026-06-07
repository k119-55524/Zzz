#include "NativeView.h"
#include "../../factories/PlatformFactory.h"

using namespace zzz::engine;

NativeView::NativeView(std::shared_ptr<IPlatform> platform) :
	m_Platform{ platform }
{
	ensure(platform != nullptr, "Platform must not be null.");

	Initialize();
}

void NativeView::Initialize()
{
	m_Window = m_Platform->GetFactory().CreateAppWin(m_Platform);
	auto res = m_Window->Initialize(m_Platform->GetAppName());
	if (!res)
		THROW_RUNTIME("Failed to initialize window: {}.", res.error());
}
