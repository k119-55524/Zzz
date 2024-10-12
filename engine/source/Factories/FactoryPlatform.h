#pragma once

#include "../Platforms/IO/IIO.h"
#include "../Platforms/MessageBox/ISysMB.h"

using namespace Zzz::Platforms;

namespace Zzz
{
	class FactoryPlatform
	{
	public:
		unique_ptr<ISysMB> GetSystemImplementationMessageBox();
		shared_ptr<IIO> GetPlatformIO();
	};
}
