#pragma once

#include "source/helpers/zResult.h"

namespace Zzz
{
	class zEngine
	{
	public:
		struct s_EngineInit
		{

		};

	public:
		zEngine();

		zResult Initialize(const s_EngineInit* const initData);

	private:

	};
}