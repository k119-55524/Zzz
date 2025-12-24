#pragma once

#include "..\..\source\BaseTest.h"

import Result;

using namespace zzz;
using namespace zzz::ztests;

namespace zzz::ztests
{
	class test_Matrix4x4 : public BaseTest<test_Matrix4x4>
	{
	public:
		test_Matrix4x4();

		bool Initialize() override;
		Result<std::string> Run() override;
		bool Kill() override;
	};
}