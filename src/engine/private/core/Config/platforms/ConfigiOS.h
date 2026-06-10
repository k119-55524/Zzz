#pragma once

#if defined(Z_IOS)

#include "../../serialize/Serializer.h"

namespace zzz::engine
{
	class ConfigiOS final : public ISerializable
	{
	public:
		ConfigiOS() = default;
		~ConfigiOS() override = default;
	};
}
#endif // defined(Z_IOS)
