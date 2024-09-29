#pragma once

#include "../Types.h"

namespace Zzz
{
	enum e_ErrorCode : zI64
	{
		Success,
		Failure,
		Warning
	};

	class zResult
	{
	public:
		zResult();
		zResult(e_ErrorCode ecode);

		bool isSuccess() const { return result == e_ErrorCode::Success; }

	private:
		e_ErrorCode result;
	};
}