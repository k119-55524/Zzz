#pragma once

#include "../Header.h"

class BaseTest
{
public:
	BaseTest() = delete;
	BaseTest(BaseTest&) = delete;
	BaseTest(BaseTest&&) = delete;

	BaseTest(string name) :
		testName{ name },
		testResult{ "Success!!!" }
	{}

	virtual bool Initialize() = 0;
	virtual bool Run() = 0;
	virtual void Shutdown() = 0;

	inline const string& GetTestName() const { return testName; };
	inline const string& GetTestResult() const { return testResult; };

protected:
	string testName;
	string testResult;
};