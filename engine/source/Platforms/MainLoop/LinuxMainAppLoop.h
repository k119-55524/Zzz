#pragma once

#include "IMainAppLoop.h"

namespace Zzz::Platforms
{
#ifdef __linux__

	class LinuxMainAppLoop : public IMainAppLoop
	{
	public:
		LinuxMainAppLoop() = delete;
		LinuxMainAppLoop(LinuxMainAppLoop&) = delete;
		LinuxMainAppLoop(LinuxMainAppLoop&&) = delete;

		LinuxMainAppLoop(unique_ptr<IWinApp> _win, unique_ptr<IGAPI> _gapi);

		~LinuxMainAppLoop() override;

		void Run() override;
	};
#endif // __linux__
}