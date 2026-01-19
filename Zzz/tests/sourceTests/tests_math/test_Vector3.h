#pragma once

#include "..\..\source\BaseTest.h"

import Result;

using namespace zzz;
using namespace zzz::ztests;

namespace zzz::ztests
{
	class test_Vector3 : public BaseTest<test_Vector3>
	{
	public:
		test_Vector3();

		bool Initialize() override;
		Result<std::string> Run() override;
		bool Kill() override;
	};
}
