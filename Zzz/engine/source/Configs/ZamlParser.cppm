
export module ZamlParser;

import ZamlParser_MSWinDirectX;

export namespace zzz
{
#if defined(ZPLATFORM_MSWINDOWS)
		export typedef ZamlParser_MSWinDirectX ZamlParser;
#elif defined(ZPLATFORM_LINUX)
#elif defined(ZPLATFORM_ANDROID)
#elif defined(ZPLATFORM_MACOS)
#elif defined(ZPLATFORM_IOS)
#else
#error ">>>>> [Compile error]. This branch requires implementation for the current platform"
#endif
}