#include "Window_Common.h"

using namespace zzz::engine;

WindowBase::WindowBase(const std::shared_ptr<Platform> platform, const std::shared_ptr<Input> input, std::function<void()> onWindowClose) :
	m_Platform{ platform },
	m_Input{ input },
	OnWindowClose{ onWindowClose }
{
	ensure(OnWindowClose != nullptr, "OnWindowClose must not be null.");
}
