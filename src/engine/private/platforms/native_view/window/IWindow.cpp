#include "IWindow.h"

using namespace zzz::engine;

IWindow::IWindow(std::shared_ptr<ConfigManager> configManager) :
	m_ConfigManager{ configManager }
{
	DOut("IWindow construction");

	ensure(m_ConfigManager != nullptr, "ConfigManager must not be null.");
}