#pragma once

#include "Tests/BaseTest.h"

class TestManager
{
public:
	~TestManager();

	void AddTest(unique_ptr<BaseTest> test);
	void RunTests();

private:
	vector<unique_ptr<BaseTest>> tests;
};