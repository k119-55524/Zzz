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

		WindowsMainAppLoop(unique_ptr<IWinApp> _win, unique_ptr<IGAPI> _gapi);

		virtual ~WindowsMainAppLoop();

		void Run() override;
	};
#endif // _WINDOWS
}
