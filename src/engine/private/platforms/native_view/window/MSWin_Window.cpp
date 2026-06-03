#include "MSWin_Window.h"

#if defined(Z_WINDOWS)

using namespace zzz::engine;

MSWin_Window::MSWin_Window(std::shared_ptr<ConfigManager> configManager) :
	IWindow(configManager)
{
	DOut("MSWin_Window construction");
}

MSWin_Window::~MSWin_Window()
{
	DOut("MSWin_Window destruction");
}

[[nodiscard]] std::expected<void, std::string> MSWin_Window::Initialize()
{
	DOut("MSWin_Window initialization: START");


	DOut("MSWin_Window initialization: END");
	return std::expected<void, std::string>();
}

#endif // defined(Z_WINDOWS)