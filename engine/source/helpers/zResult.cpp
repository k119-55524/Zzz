#include "pch.h"
#include "zResult.h"

using namespace Zzz;

zResult::zResult() :
	result{ e_ErrorCode::Success },
	description{ L"" }
{
}

zResult::zResult(e_ErrorCode ecode) :
	result{ ecode },
	description{ L"" }
{
}

Zzz::zResult::zResult(e_ErrorCode ecode, const zStr& descr) :
	result{ ecode },
	description{ descr }
{
}
