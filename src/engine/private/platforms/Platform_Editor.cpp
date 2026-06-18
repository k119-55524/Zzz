#include "Platform.h"
#include "window/WinEditor.h"

using namespace zzz::engine;

void Platform::ShutdownPlatformSpecific()
{
	// Для редактора (WPF) нам не нужно снимать регистрацию с классов окна ОС.
}

void Platform::InitializePlatformSpecific()
{
	// Для редактора (WPF) нам не нужно регистрировать WNDCLASS.
	// Окном полностью управляет WPF, движок получает лишь готовый HWND.
}
