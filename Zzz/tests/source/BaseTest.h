
#pragma once
#include "Header.h"

#include <string>

namespace zzz::ztests
{
	class BaseTest
	{
		Z_NO_CREATE_COPY(BaseTest);

	public:
		BaseTest(std::string name) :
			testName{ name },
			testResult{ "Success!!!" }
		{
		}

		virtual bool Initialize() = 0;
		virtual bool Run() = 0;
		virtual void Shutdown() = 0;

		inline const std::string& GetTestName() const { return testName; };
		inline const std::string& GetTestResult() const { return testResult; };

	protected:
		std::string testName;
		std::string testResult;
	};
}