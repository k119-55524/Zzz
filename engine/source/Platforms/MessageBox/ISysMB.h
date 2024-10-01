#pragma once

#include "../../Types.h"

namespace Zzz::Platforms
{
	enum e_MessageBoxResult : zI64
	{
		OK,
		No,
		Cancel,
	};

	class ISysMB
	{
	public:
		virtual void ShowError(const zStr& message) = 0;
	};
}