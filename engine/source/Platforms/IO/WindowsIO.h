#pragma once

#include "IIO.h"

namespace Zzz::Platforms
{
	class WindowsIO : public IIO
	{
	public:
		WindowsIO();
		~WindowsIO() override;
	};
}
