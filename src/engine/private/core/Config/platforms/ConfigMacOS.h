#pragma once

#if defined(Z_MACOS)

#include "IConfig.h"

namespace zzz::engine
{
	class ConfigMacOS final : public IConfig
	{
	public:
		ConfigMacOS() = default;
		~ConfigMacOS() override = default;
	};
}
#endif // defined(Z_MACOS)
