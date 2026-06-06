#include "IWindow.h"

using namespace zzz::engine;

IWindow::IWindow(const std::shared_ptr<IPlatform> platform) :
	m_Platform{ platform }
{
}