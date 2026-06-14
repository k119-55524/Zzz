#pragma once

#include <common/common.h>

namespace zzz::logger
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