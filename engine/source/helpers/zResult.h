#pragma once

#include "../Types/Types.h"

namespace Zzz
{
	enum e_ErrorCode : zU64
	{
		eSuccess,
		eFailure,
		eWarning
	};

	class zResult
	{
	public:
		zResult();
		zResult(e_ErrorCode ecode);
		zResult(e_ErrorCode ecode, const zStr& descr);

		bool isSuccess() const { return result == e_ErrorCode::eSuccess; }

		inline const zStr& GetDescription() const { return description; };

	private:
		e_ErrorCode result;
		zStr description;
	};
}