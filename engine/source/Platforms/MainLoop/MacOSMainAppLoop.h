#pragma once

#include "IMainAppLoop.h"

namespace Zzz::Platforms
{
#if defined(__APPLE__) && !TARGET_OS_IOS
#include <Cocoa/Cocoa.h>

	class MacOSMainAppLoop : public IMainAppLoop
	{
	public:
		MacOSMainAppLoop() = delete;
		MacOSMainAppLoop(MacOSMainAppLoop&) = delete;
		MacOSMainAppLoop(MacOSMainAppLoop&&) = delete;

		MacOSMainAppLoop(unique_ptr<IWinApp> _win, unique_ptr<IGAPI> _gapi);

		virtual ~MacOSMainAppLoop();

		void Run() override;
	};
#endif // defined(__APPLE__) && !TARGET_OS_IOS
}
