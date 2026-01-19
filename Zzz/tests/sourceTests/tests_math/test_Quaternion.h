#pragma once

#include "..\..\source\BaseTest.h"

import Result;

using namespace zzz;
using namespace zzz::ztests;

namespace zzz::ztests
{
	class test_Quaternion : public BaseTest<test_Quaternion>
	{
	public:
		test_Quaternion();

		bool Initialize() override;
		Result<std::string> Run() override;
		bool Kill() override;
	};
}
