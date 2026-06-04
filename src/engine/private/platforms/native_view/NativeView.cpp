#include "NativeView.h"

using namespace zzz::engine;

NativeView::NativeView(const std::string_view appName, const EngineConfig& config) :
	m_Config{ config }
{
	Initialize(appName);
}

void NativeView::Initialize(const std::string_view appName)
{
	m_Window = m_Factory.CreateAppWin(m_Config);
	auto res = m_Window->Initialize(appName);
	if (!res)
		THROW_RUNTIME("Failed to initialize window: {}.", res.error());
}
