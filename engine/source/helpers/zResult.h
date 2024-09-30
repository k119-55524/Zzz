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
		zResult(e_ErrorCode ecode, const zStr& descr);

		bool isSuccess() const { return result == e_ErrorCode::Success; }

		inline const zStr& GetDescription() const { return description; };

	private:
		e_ErrorCode result;
		zStr description;
	};
}