#pragma once

#include <foundation.h>

namespace zzz
{
	enum class eLogMessageType : zU8
	{
		Message,
		Warning,
		Error,
		Exception,
		Critical,
		Fatal
	};
}