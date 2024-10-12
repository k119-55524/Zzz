#pragma once

#include "IMainAppLoop.h"

namespace Zzz::Platforms
{
#ifdef __ANDROID__

	class AndroidMainAppLoop : public IMainAppLoop
	{
	public:
		AndroidMainAppLoop() = delete;
		AndroidMainAppLoop(AndroidMainAppLoop&) = delete;
		AndroidMainAppLoop(AndroidMainAppLoop&&) = delete;

		AndroidMainAppLoop(unique_ptr<IWinApp> _win, unique_ptr<IGAPI> _gapi);

		~AndroidMainAppLoop() override;

		void Run() override;
	};

#endif // __ANDROID__
}