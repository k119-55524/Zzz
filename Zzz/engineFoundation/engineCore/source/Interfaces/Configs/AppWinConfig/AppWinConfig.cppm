
export module AppWinConfig;

import AppWinConfig_MSWin;

export namespace zzz::core
{
#if defined(ZPLATFORM_MSWINDOWS)
	export typedef AppWinConfig_MSWin AppWinConfig;
#elif defined(ZPLATFORM_LINUX)
#elif defined(ZPLATFORM_ANDROID)
#elif defined(ZPLATFORM_MACOS)
#elif defined(ZPLATFORM_IOS)
#else
#error ">>>>> [Compile error]. This branch requires implementation for the current platform"
#endif
}