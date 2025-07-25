
#include "pch.h"
export module engine;

import result;
using namespace zzz;
using namespace zzz::result;

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
