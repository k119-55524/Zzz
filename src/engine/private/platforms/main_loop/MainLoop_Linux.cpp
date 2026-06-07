#include "MainLoop_Linux.h"
#include "../platforms/PlatformLinux.h"

#if defined(Z_LINUX)

#include <poll.h>

using namespace zzz::engine;

MainLoop_Linux::MainLoop_Linux(const std::shared_ptr<IPlatform> platform) :
	IMainLoop(platform),
	m_Display{ nullptr }
{
	std::shared_ptr<PlatformLinux> platformLinux = std::dynamic_pointer_cast<PlatformLinux>(m_Platform);
	ensure(platformLinux != nullptr, "Platform is not PlatformLinux.");

	m_Display = platformLinux->GetDisplay();
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

		onUpdateSystem();
	}
}
#endif // defined(Z_LINUX)