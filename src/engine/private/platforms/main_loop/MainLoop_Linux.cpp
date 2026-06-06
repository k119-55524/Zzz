#include "MainLoop_Linux.h"
#include "../platforms/PlatformLinux.h"

#if defined(Z_LINUX)

using namespace zzz::engine;

MainLoop_Linux::MainLoop_Linux(const std::shared_ptr<IPlatform> platform) :
	IMainLoop(platform),
	m_Display{ nullptr },
	m_Running{ true }
{
	std::shared_ptr<PlatformLinux> platformLinux = std::dynamic_pointer_cast<PlatformLinux>(m_Platform);
	ensure(platformLinux != nullptr, "Platform is not PlatformLinux.");

	m_Display = platformLinux->GetDisplay();
	ensure(m_Display != nullptr, "Display is not PlatformLinux.");
}

void MainLoop_Linux::Run()
{
	while (m_Running)
	{
		wl_display_dispatch_pending(m_Display);

		onUpdateSystem();

		wl_display_flush(m_Display);
	}
}
#endif // defined(Z_LINUX)