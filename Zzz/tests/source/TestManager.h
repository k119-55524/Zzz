#pragma once

#include <vector>
#include <memory>

#include "BaseTest.h"

using namespace zzz::ztests;

namespace zzz::ztests
{
	class TestManager
	{
	public:

		void AddTest(std::unique_ptr<BaseTest> test);
		void RunTests();

	private:
		std::vector<std::unique_ptr<BaseTest>> tests;
	};
}