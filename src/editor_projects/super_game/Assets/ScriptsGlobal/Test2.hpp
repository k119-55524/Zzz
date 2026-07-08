#pragma once
#include <Game.h>

class Test2 : public zzz::script::Game
{
public:
	Test2();

	void OnStart() override;
	void OnUpdate(float deltaTime) override;

};

