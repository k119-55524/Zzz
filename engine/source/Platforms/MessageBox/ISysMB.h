#pragma once

#include "../../Types/Types.h"

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
		ISysMB() = default;

		virtual ~ISysMB() = 0;

		virtual void ShowError(const zStr& message) = 0;
	};
}