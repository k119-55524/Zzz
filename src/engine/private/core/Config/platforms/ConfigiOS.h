#pragma once


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
 // defined(Z_IOS)
