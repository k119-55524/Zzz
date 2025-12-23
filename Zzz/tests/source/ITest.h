#pragma once

#include <string>

class ITest
{
public:
	virtual ~ITest() = default;

	virtual bool Initialize() = 0;
	virtual bool Run() = 0;
	virtual bool Kill() = 0;

	virtual constexpr std::string_view GetTestName() const = 0;
	virtual const std::string& GetTestResult() const = 0;
};
