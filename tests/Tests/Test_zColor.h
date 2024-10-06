#pragma once

#include "BaseTest.h"

class Test_zColor : public BaseTest
{
public:
	Test_zColor();

	bool Initialize() override;
	bool Run() override;
	void Shutdown() override;
};

