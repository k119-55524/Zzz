#pragma once

#include "BaseTest.h"

class Test_Engine : public BaseTest
{
public:
	Test_Engine();

	bool Initialize() override;
	bool Run() override;
	void Shutdown() override;

private:
	unique_ptr<zEngine> engine;
};