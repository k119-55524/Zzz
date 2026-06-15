#include "MainLoop_Linux.h"
#include "../Platform.h"


#include <poll.h>

using namespace zzz::engine;

MainLoop_Linux::MainLoop_Linux(const std::shared_ptr<Platform> platform, std::function<void()> onUpdate) :
	MainLoopBase(platform, std::move(onUpdate)),
	m_Display{ nullptr }
{
	m_Display = m_Platform->GetNativeData()->display;
	ensure(m_Display != nullptr, "Display is not PlatformLinux.");
}

void MainLoop_Linux::Run()
{
	while (isRunning)
	{
		// Flush pending requests; dispatch any events already in the queue
		while (wl_display_prepare_read(m_Display) != 0)
			wl_display_dispatch_pending(m_Display);

		wl_display_flush(m_Display);

		// Non-blocking check: read new events only if they are available
		pollfd pfd{ wl_display_get_fd(m_Display), POLLIN, 0 };
		if (poll(&pfd, 1, 0) > 0)
			wl_display_read_events(m_Display);
		else
			wl_display_cancel_read(m_Display);

		if (wl_display_dispatch_pending(m_Display) == -1)
			break;

		OnUpdate();
	}
}
