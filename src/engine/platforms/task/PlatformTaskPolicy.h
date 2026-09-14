#pragma once

#include "core/utils/Defines.h"
#include "platforms/PlatformTaskPolicyDesktop.h"
#include "platforms/PlatformTaskPolicyMobile.h"

namespace zzz::engine
{
#if Z_MOBILE
	using PlatformTaskPolicy = PlatformTaskPolicyMobile;
#else
	using PlatformTaskPolicy = PlatformTaskPolicyDesktop;
#endif
}
