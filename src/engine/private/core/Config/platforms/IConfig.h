#pragma once

#include "../../serialize/Serializer.h"

namespace zzz::engine
{
	class IConfig : public ISerializable
	{
	public:
		virtual ~IConfig() = default;
	};
}
