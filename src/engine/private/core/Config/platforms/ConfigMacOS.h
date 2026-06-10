#pragma once


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
 // defined(Z_MACOS)
