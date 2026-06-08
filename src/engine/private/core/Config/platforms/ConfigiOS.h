#pragma once

#if defined(Z_IOS)

#include "IConfig.h"

namespace zzz::engine
{
	class ConfigiOS final : public IConfig
	{
	public:
		ConfigiOS() = default;
		~ConfigiOS() override = default;
	};
}
#endif // defined(Z_IOS)
