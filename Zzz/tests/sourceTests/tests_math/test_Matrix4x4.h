#pragma once

#include "..\..\source\BaseTest.h"

import Matrix4x4;

//using namespace zzz::math;
using namespace zzz::ztests;

namespace zzz::ztests
{
	class test_Matrix4x4 :
		public BaseTest
	{
	public:
		test_Matrix4x4();

		bool Initialize() override;
		bool Run() override;
		void Shutdown() override;

	private:
		//bool Test(zVec2& v, float x, float y, std::string mes = "");
	};
}