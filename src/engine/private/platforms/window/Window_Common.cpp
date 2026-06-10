#include "Window_Common.h"

using namespace zzz::engine;

WindowBase::WindowBase(const std::shared_ptr<Platform> platform, const std::shared_ptr<Input> input) :
	m_Platform{ platform },
	m_Input{ input }
{
}
