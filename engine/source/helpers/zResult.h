#pragma once

namespace Zzz
{
	class zResult
	{
		enum e_ErrorCode : int
		{
			Success,
			Failure,
			Warning
		};

	public:
		zResult();

		bool isSuccess() const { return result == e_ErrorCode::Success; }

	private:
		e_ErrorCode result;
	};
}