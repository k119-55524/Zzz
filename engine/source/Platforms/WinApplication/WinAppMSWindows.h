#pragma once

#include "WinAppBase.h"

#if defined(_WINDOWS)
namespace Zzz::Platforms
{
	class WinAppMSWindows : public WinAppBase
	{
	protected:
		zResult Initialize(const s_zWinCreateSetting* data) override;
	};
}
#endif // defined(_WINDOWS)