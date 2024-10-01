#pragma once

#include "../Platforms/MessageBox/ISysMB.h"

using namespace Zzz::Platforms;

namespace Zzz
{
	class FactoryPlatform
	{
	public:
		unique_ptr<ISysMB> GetSystemImplementationMessageBox();
	};
}
