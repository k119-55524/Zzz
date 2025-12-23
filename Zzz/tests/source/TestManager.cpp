
#include "TestManager.h"

void TestManager::AddTest(std::unique_ptr<BaseTest> test)
{
	tests.push_back(std::move(test));
}

void TestManager::RunTests()
{
	std::cout << "===>>> START TESTS. Tests counts: " << tests.size() << ". <<<===" << std::endl << std::endl;

	for (auto& test : tests)
	{
		if (test->Initialize())
		{
			std::cout << "Test " << test->GetTestName() << " initialized successfully." << std::endl;

			if (!test->Run())
				std::cout << "#### Test failure!!! ###" << std::endl;
			std::cout << "Test " << test->GetTestName() << " result: " << test->GetTestResult() << std::endl;

			test->Shutdown();
			std::cout << "Test " << test->GetTestName() << " shutdown successfully." << std::endl << std::endl;
		}
		else
		{
			std::cout << "Failed to initialize test:" << test->GetTestName() << "." << std::endl << std::endl;
		}
	}

	std::cout << "===>>> END TESTS. Tests counts: " << tests.size() << ". <<<===" << std::endl << std::endl;
}