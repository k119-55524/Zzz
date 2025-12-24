#pragma once

#include <string>

import Result;

using namespace zzz;

class ITest
{
public:
	virtual ~ITest() = default;

	virtual bool Initialize() = 0;
	virtual Result<std::string> Run() = 0;
	virtual bool Kill() = 0;

	virtual constexpr std::string_view GetTestName() const = 0;
};
