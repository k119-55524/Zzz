#pragma once

#include "IMainAppLoop.h"

namespace Zzz::Platforms
{
#ifdef _WINDOWS
	class WindowsMainAppLoop : public IMainAppLoop
	{
	public:
		WindowsMainAppLoop() = delete;
		WindowsMainAppLoop(WindowsMainAppLoop&) = delete;
		WindowsMainAppLoop(WindowsMainAppLoop&&) = delete;

		WindowsMainAppLoop(function<void()> _updateSystem);

		void Run() override;
	};
#endif // _WINDOWS
}
