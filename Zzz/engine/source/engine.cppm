
#include "pch.h"
export module engine;

import zzz;
using namespace zzz;
using namespace zzz::error;

export namespace zzz
{
	export class engine
	{
	public:
		zResult<> initialize();
		zResult<> go();

	private:

	};
}

zResult<> engine::initialize()
{
	//return Unexpected(zResult::failure);
	//return Unexpected(zResult::failure, "Error");
	return eResult::success;
}

zResult<> engine::go()
{
	return eResult::success;
}
