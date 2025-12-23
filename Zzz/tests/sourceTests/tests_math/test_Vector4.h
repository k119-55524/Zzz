#pragma once

#include "..\..\source\BaseTest.h"

using namespace zzz::ztests;

namespace zzz::ztests
{
	class test_Vector4 : public BaseTest<test_Vector4>
	{
	public:
		test_Vector4();

		bool Initialize() override;
		bool Run() override;
		bool Kill() override;

	private:
		//bool Test(zVec2& v, float x, float y, std::string mes = "");
	};
}
