#include "NativeView.h"

using namespace zzz::engine;

NativeView::NativeView(std::shared_ptr<ConfigManager> configManager) :
	m_ConfigManager{ configManager }
{
	ensure(m_ConfigManager != nullptr, "ConfigManager must not be null.");

	Initialize();
}

NativeView::~NativeView()
{
	DOut("NativeView destruction");
}

void NativeView::Initialize()
{
	DOut("NativeView initialization: START");

	m_Window = m_PlatformFactory.CreateAppWin(m_ConfigManager);
	auto res = m_Window->Initialize();
	if (!res)
		THROW_RUNTIME("Failed to initialize window: {}.", res.error());

	DOut("NativeView initialization: END");
}