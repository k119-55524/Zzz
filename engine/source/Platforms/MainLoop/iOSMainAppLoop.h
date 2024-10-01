#pragma once

#include "IMainAppLoop.h"

namespace Zzz::Platforms
{
#if defined(__APPLE__) && TARGET_OS_IOS
class iOSMainAppLoop : public IMainAppLoop
{
public:
	iOSMainAppLoop() = delete;
	iOSMainAppLoop(iOSMainAppLoop&) = delete;
	iOSMainAppLoop(iOSMainAppLoop&&) = delete;

	iOSMainAppLoop(unique_ptr<IWinApp> _win, unique_ptr<IGAPI> _gapi);

	virtual ~iOSMainAppLoop();

	void Run() override;
};
#endif // defined(__APPLE__) && !TARGET_OS_IOS
}