#if defined(Z_LINUX)

#include "Linux_Window.h"

using namespace zzz::engine;

Linux_Window::Linux_Window(const EngineConfig& config) :
	IWindow(config)
{
}

[[nodiscard]] std::expected<void, std::string> Linux_Window::Initialize(const std::string_view appName)
{
	return std::expected<void, std::string>();
}
#endif // defined(Z_LINUX)