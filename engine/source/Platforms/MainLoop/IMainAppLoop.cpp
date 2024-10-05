#include "pch.h"
#include "IMainAppLoop.h"

using namespace Zzz::Platforms;

IMainAppLoop::IMainAppLoop(function<void()> _updateSystem) :
	updateSystem{_updateSystem}
{
}

IMainAppLoop::~IMainAppLoop()
{
}
