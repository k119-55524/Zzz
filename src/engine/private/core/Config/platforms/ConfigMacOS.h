#pragma once

#if defined(Z_MACOS)

#include "../../serialize/Serializer.h"

namespace zzz::engine
{
	class ConfigMacOS final : public ISerializable
	{
	public:
		ConfigMacOS() = default;
		~ConfigMacOS() override = default;
	};
}
#endif // defined(Z_MACOS)
