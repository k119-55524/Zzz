#include "IWindow.h"

using namespace zzz::engine;

IWindow::IWindow(const std::shared_ptr<IPlatform> platform, const std::shared_ptr<IInput> input) :
	m_Platform{ platform },
	m_Input{ input }
{
}
