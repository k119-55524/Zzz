#include "pch.h"
#include "zResult.h"

using namespace Zzz;

zResult::zResult() :
	result{ e_ErrorCode::Success }
{
}

zResult::zResult(e_ErrorCode ecode) :
	result{ ecode }
{
}
