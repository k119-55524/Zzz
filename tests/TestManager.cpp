
#include "TestManager.h"

TestManager::~TestManager()
{
	for (auto& test : tests)
	{
		if (test != nullptr)
			test.reset(nullptr);
	}

	tests.clear();
}

void TestManager::AddTest(unique_ptr<BaseTest> test)
{
	tests.push_back(move(test));
}

void TestManager::RunTests()
{
	cout << "===>>> START TESTS. Tests counts: " << tests.size() << ". <<<===" << endl << endl;

	for (auto& test : tests)
	{
		if (test->Initialize())
		{
			cout << "Test " << test->GetTestName() << " initialized successfully." << endl;

			if (!test->Run())
				cout << "#### Test failure!!! ###" << endl;
			cout << "Test " << test->GetTestName() << " result: " << test->GetTestResult() << endl;

			test->Shutdown();
			cout << "Test " << test->GetTestName() << " shutdown successfully." << endl << endl;
		}
		else
		{
			cout << "Failed to initialize test:" << test->GetTestName() << "." << endl << endl;
		}
	}

	cout << "===>>> END TESTS. Tests counts: " << tests.size() << ". <<<===" << endl << endl;
}