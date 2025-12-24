
#include "TestManager.h"

void TestManager::AddTest(std::unique_ptr<ITest> test)
{
	tests.push_back(std::move(test));
}

void TestManager::RunTests()
{
	size_t passedCount = 0;
	size_t failedCount = 0;
	size_t notInitializedCount = 0;
	size_t notShutdownCount = 0;

	std::cout << "\033[33m===>>> START TESTS. Tests counts: " << tests.size() << ". <<<===\033[0m\n\n";

	size_t testIndex = 1;
	for (auto& test : tests)
	{
		std::cout << "\033[36mTest " << testIndex << " of " << tests.size() << ".\033[0m\n";
		if (test->Initialize())
		{
			std::cout << "\033[34m[" << test->GetTestName() << "]\033[0m\n";
			std::cout << "  Init result: \033[32mSuccess.\033[0m" << std::endl;

			auto res = test->Run();
			if (!res)
			{
				std::wcout << "  Test result: \033[31mFailure: \033[35m" << res.error().getMessage() << "\033[0m\n";
				failedCount++;
			}
			else
			{
				std::cout << "  Test result: \033[32m" << res.value() << "\033[0m\n";
				passedCount++;
			}

			if(test->Kill())
				std::cout << "  Kill result: \033[32mSuccess.\033[0m\n\n";
			else
			{
				std::cout << "  Kill result: \033[31mFailure.\033[0m\n\n";
				notShutdownCount++;
			}
		}
		else
		{
			std::cout << "\033[34m[" << test->GetTestName() << "]\033[31m - Failed init.\033[0m\n\n";
			notInitializedCount++;
			failedCount++;
		}

		testIndex++;
	}

	std::cout << "\033[33m===>>>    END TESTS    <<<===\033[0m\n\n";

	std::cout << "\033[32mPassed:      " << passedCount << " of " << tests.size() << "\033[0m\n";

	if (failedCount > 0)
		std::cout << "\033[31mFailed:      " << failedCount << "\033[0m\n";

	if (notInitializedCount > 0)
		std::cout << "\033[31m - Not init: " << notInitializedCount << "\033[0m\n";

	if (notShutdownCount > 0)
		std::cout << "\033[31m - Not kill: " << notShutdownCount << "\033[0m\n";

	std::cout << "\n\033[33m===>>>   EXIT  TESTS   <<<===\033[0m\n\n";
}